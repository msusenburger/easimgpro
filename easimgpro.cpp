#include <iostream>
#include <fstream>
#include <sstream>   
#include <string>
#include <cmath>

/*
Copyright (c) 2018 Markus Susenburger

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial 
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE 
AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
*/ 

typedef struct _pgm_image
{
    char magic_nummber[2];
    int width;
    int height;
    int MAXVAL;
    float *image;
}pgm_image;

/*
Apply box blur to the result image. 
Formula from https://en.wikipedia.org/wiki/Box_blur.
*/
int smoothness(pgm_image *in, pgm_image *res)
{
    float *in_array = in->image;
    float *out_array = res->image;
	
	int height = in->height;

    float cov_matrix[3][3] = {{.111,.111,.111}, {.111,.111,.111}, {.111,.111,.111}};

    //float cov_matrix[3][3] = {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}};	

    for (int x = 0; x < in->width; x++)
    {
        for (int y = 0; y < in->height; y++)
        {
            if (x == 0 || y == 0 || x == in->width -1 || y == in->height -1)
			{
				out_array[x * height + y] = in_array[x * height + y];
				continue;
			}
			float sum = 0;
            for( int i = 1; i <= 3; i++)
            {
                for (int j = 1; j <= 3; j++)
                {
                    sum += cov_matrix[i-1][j-1] * in_array[(x + i - 2) * height + (y + j - 2)];
                }
            }
            out_array[x * height + y] = std::floor(sum) > 0 ? std::floor(sum) : 0;
        }
    }
    return 1;
}

void save_image(pgm_image * to_save, std::string comment)
{
	static int counter = 0;
	std::stringstream s;
	s << "easyimgpro_" << counter << ".pgm";

	std::ofstream outfile(s.str());

	outfile << "P2" << std::endl;
	outfile << "#" << comment << std::endl;
	outfile << to_save->width << ' ' << to_save->height << std::endl;
	outfile << to_save->MAXVAL << std::endl;

	for (int x = 0; x < to_save->height * to_save->width; x++)
		outfile << to_save->image[x] << " ";
	outfile.close();

	counter++;
}

int main(int argc, char *argv[])
{
	if (argc > 2)
	{
		std::cout << "Wrong number of arguments." << std::endl;
		return -1;
	}
	const char * filename;
	if (argc == 2)
	{
		filename = argv[1];
	}	
	else
	{
		filename = "baboon.ascii.pgm";
	}

	std::ifstream infile(filename);		

	if ((infile.rdstate() & infile.failbit))
	{
		std::cout << "Error in file reading - does the file exist?" << std::endl;
		return -1;
	}
   	std::string line;

   	pgm_image *image = new pgm_image; // pointer to a new pgm image

   	std::getline(infile, line);
   
	if (line[0] == 'P' && line[1] == '2')
	{
		image->magic_nummber[0] == line[0];
		image->magic_nummber[1] == line[1];
	}
	else
	{
		std::cout << "Wrong file format specifier. Is it an ASCII coded Pixel Graymap (.pgm)?" << line[0] << line[1] << std::endl;
		delete(image);
		return -1;
	}

	std::getline(infile, line); // ignore comment
	std::getline(infile, line);

	int a,b;
	a = std::stoi(line);
	b = line.find(' ');
	b = std::stoi(&line[b]);

	image->width = a;
	image->height = b;

	std::getline(infile, line);
	image->MAXVAL = std::stoi(line);
 
   	pgm_image result;
	pgm_image diff;

	image->image 	= new float[image->width * image->height];
	result.image 	= new float[image->width * image->height];
	diff.image 		= new float[image->width * image->height];

	result.magic_nummber[0] = image->magic_nummber[0];
	result.magic_nummber[1] = image->magic_nummber[1];	
	result.height 			= image->height;
	result.width 			= image->width;
	result.MAXVAL 			= image->MAXVAL;

	diff.magic_nummber[0] 	= image->magic_nummber[0];
	diff.magic_nummber[1] 	= image->magic_nummber[1];	
	diff.height 			= image->height;
	diff.width 				= image->width;
	diff.MAXVAL 			= image->MAXVAL;

	int x = 0, y = 0;
	while(std::getline(infile, line))
	{
		for (int n = 0; n < line.length();)
		{
			if (line[n] >= '0' && line[n] <= '9')
			{
				image->image[x * image->height + y++] = std::stoi(&line[n]);
				while (n < line.length() && line[n] >= '0' && line[n] <= '9')
				{
					n++;
				}
			} 
			else
				n++;
			if (y == image->height)
			{
				y = 0;
				x++;
			}
		}
	}

	save_image(image, "This is a the original image.");
   	
	// blur with box blur
	smoothness(image, &result);
    
	save_image(&result, "This is the smoothened image.");
	
	// build a difference image.
	for (int i = 0; i < diff.width * diff.height; i++)
	{
		diff.image[i] = (result.image[i] - image->image[i] < 0 ) ? 255 - result.image[i] + image->image[i] : result.image[i] - image->image[i];
	}
	save_image(&diff, "A difference image, possibly black and white coded?");
	
	// free all memory.
	delete [] result.image;
	delete [] diff.image;
	delete [] image->image;
	delete image;
}