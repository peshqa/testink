/*
ppm_image_loader.h - Bitmap Picture file format loader
file format info taken from https://en.wikipedia.org/wiki/BMP_file_format
data is stored "bottom-to-top", so it is inverted while reading
2023/12/28, peshqa
*/

#pragma once

#include <fstream>
#include <string>

#pragma pack(push, 1)
typedef struct
{
	unsigned short magic_number;
	unsigned int file_size;
	unsigned short reserved1;
	unsigned short reserved2;
	unsigned int offset;
	// start of BITMAPINFOHEADER
	unsigned int header_size;
	int width;
	int height;
	unsigned short color_planes; // always 1
	unsigned short bits_per_pixel;
	unsigned int compression_method; // BI_RGB is 0 (most common)
	unsigned int raw_bmp_size;
	int horiz_resolution; // pixel per metre (?)
	int vertical_resolution; // pixel per metre (?)
	unsigned int num_colors;
	unsigned int num_important_colors; // ?
} BitmapFileHeader;
#pragma pack(pop)

int LoadBMPImage(SharedState *s, std::string file_path, SimpleImage *image)
{
	if (OpenAssetFileA(s, file_path) != 0)
	{
		ASSERT(!"LoadBMPImage - Couldn't open the file");
		return 1;
	}
	
	//char *buffer = new char[sizeof(BitmapFileHeader)]{};
	BitmapFileHeader header{};
	ReadAssetBytesA(s, (char*)&header, sizeof(BitmapFileHeader));
	
	if (header.magic_number != 'MB')
	{
		ASSERT(!"LoadBMPImage - Magic number not matching");
		return 2;
	}
	
	if (header.offset < 54 || header.bits_per_pixel < 32 || header.compression_method != 0)
	{
		ASSERT(!"LoadBMPImage - Unrecognized bmp file format");
		return 3;
	}

	if (image->pixels != 0)
	{
		delete [] image->pixels;
	}
	image->pixels = new int[header.width*header.height]{};
	image->width = header.width;
	image->height = header.height;
	
	char *buffer = new char[header.width*header.height*4]{};

	ReadAssetBytesA(s, buffer, header.width*header.height*4);
	for (int y = 0; y < header.height; y++)
	{
		for (int x = 0; x < header.width; x++)
		{
			image->pixels[(header.height-y-1)*header.width+x] = MakeColor(255,
				buffer[y*header.width*4+x*4],
				buffer[y*header.width*4+x*4+1],
				buffer[y*header.width*4+x*4+2]
				);
		}
	}
	
	delete [] buffer;
	
	CloseAssetFile(s);
	return 0;
}
