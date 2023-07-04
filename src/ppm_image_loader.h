/*
ppm_image_loader.h - a simple tool to load a file wtih a simple image format

Initially I wanted to implement a png loader, but then I encountered this comment 
on stack overflow:
"I would strongly advise you to look at a much simpler file format first. [...]
Start with an easier image format. ppm is a deliberately simple format, good to start with. 
tga, old but easy, introduces you to several more concepts such as bit depths and color mapping. 
Microsoft's bmp has some nice little caveats but can still be considered 'beginner friendly'. 
If you are interested in simple compression, the basic Run Length Encoding of a pcx is a good starting point. 
After mastering that you could look in to the gif format, which uses the much harder LZW compression."

2023/07/01, peshqa
*/

/*
A PPM file consists of a sequence of one or more PPM images. There are no data, delimiters, or padding before, after, or between images.

Each PPM image consists of the following:

1. A "magic number" for identifying the file type. A ppm image's magic number is the two characters "P6".
2. Whitespace (blanks, TABs, CRs, LFs).
3. A width, formatted as ASCII characters in decimal.
4. Whitespace.
5. A height, again in ASCII decimal.
6. Whitespace.
7. The maximum color value (Maxval), again in ASCII decimal. Must be less than 65536 and more than zero.
8. A single whitespace character (usually a newline).
9. A raster of Height rows, in order from top to bottom. Each row consists of Width pixels, in order from left to right. Each pixel is a triplet of red, green, and blue samples, in that order. Each sample is represented in pure binary by either 1 or 2 bytes. If the Maxval is less than 256, it is 1 byte. Otherwise, it is 2 bytes. The most significant byte is first.
*/

#include <iostream>
#include <fstream>
#include <string>

typedef struct
{
	int width;
	int height;
	// assuming its 4 bytes per pixel
	int *pixels;
} SimpleImage;

int TerminatePPMImage(SimpleImage *image)
{
	delete [] image->pixels;
	return 0;
}

int LoadPPMImage(const char file_path[], SimpleImage *image)
{
	unsigned char sample{};
	
	FILE *file;
	file = fopen(file_path, "rb");
	
	if (file == 0)
	{
		return 1;
	}
	
	// TODO: completely redo this terrible code below
	// perhaps implementing a parser is required
	
	char buffer[255]{};
	
	fread(buffer, 1, 3, file);
	fread(buffer, 1, 4, file);
	fread(buffer, 1, 4, file);
	fread(buffer, 1, 4, file);
	
	int width = 201;
	int height = 200;
	int max_value = 255;
	
	image->width = width;
	image->height = height;
	
	if (image->pixels != 0)
	{
		delete [] image->pixels;
	}
	image->pixels = new int[width*height]{};
	
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int pixel = 0;
			fread(&sample, 1, 1, file);
			pixel += sample << 16;
			fread(&sample, 1, 1, file);
			pixel += sample << 8;
			fread(&sample, 1, 1, file);
			pixel += sample;
			image->pixels[y*width+x] = pixel;
		}
	}
	
	fclose(file);
	return 0;
}