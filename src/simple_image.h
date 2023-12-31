/*
ppm_image_loader.h - this header file combines different file format loaders
2023/12/28, peshqa
*/
#pragma once

#include <fstream>
#include <string>

// define some common stuff
typedef struct
{
	int width;
	int height;
	// assuming its 4 bytes per pixel
	int *pixels;
} SimpleImage;

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

int TerminateImage(SimpleImage *image)
{
	delete [] image->pixels;
	return 0;
}

int LoadPPMImage(SharedState *s, std::string file_path, SimpleImage *image)
{
	
	//std::ifstream file(file_path, std::ifstream::binary);
	
	if (OpenAssetFileA(s, file_path) != 0)
	{
		ASSERT(!"LoadPPMImage - Couldn't open the file");
		return 1;
	}
	
	std::string line;
	ReadAssetLineA(s, line);
	//file >> magic_number;
	
	//if (line != "P6")
	if (line[0] != 'P' && line[1] != '6')
	{
		ASSERT(!"LoadPPMImage - Magic number not matching");
		return 2;
	}
	
	unsigned int width;
	unsigned int height;
	int max_value;
	ReadAssetUntilSpaceA(s, line);
	width = stoi(line);
	ReadAssetUntilSpaceA(s, line);
	height = stoi(line);
	ReadAssetUntilSpaceA(s, line);
	max_value = stoi(line);
	//file >> width >> height >> max_value;
	
	if (max_value > 255)
	{
		return 3;
	}
	
	//file.ignore();
	char ignored;
	ReadAssetBytesA(s, &ignored, 1);
	image->width = width;
	image->height = height;
	
	if (image->pixels != 0)
	{
		delete [] image->pixels;
	}
	image->pixels = new int[width*height]{};
	
	char *buffer = new char[width*height*3]{};
	//file.read(buffer, width*height*3);
	ReadAssetBytesA(s, buffer, width*height*3);
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			/*int pixel = 0;
			pixel += buffer[y*width*3+x*3] << 16;
			pixel += buffer[y*width*3+x*3+1] << 8;
			pixel += buffer[y*width*3+x*3+2];
			
			image->pixels[y*width+x] = pixel;*/
			image->pixels[y*width+x] = MakeColor(255,
				buffer[y*width*3+x*3]/* << 16*/,
				buffer[y*width*3+x*3+1]/* << 8*/,
				buffer[y*width*3+x*3+2]
				);
		}
	}
	
	delete [] buffer;
	//file.close();
	CloseAssetFile(s);
	return 0;
}

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

int SampleTexture(SimpleImage *img, float u, float v)
{
	if (u >= 0.0f && u <= 1.0f && v >= 0.0f && v <= 1.0f)
	{
		return img->pixels[img->width*(int)((img->height-1)*(1.0f-v)) + (int)(img->width*u)];
	}
	return MakeColor(255, 241, 87, 236);
}

void DrawImage(PlatformBitBuffer *bitBuff, SimpleImage *image, int min_x, int min_y)
{
	for (int j = 0; j < image->height; j++)
	{
		for (int i = 0; i < image->width; i++)
		{
			PlatformDrawPixel(bitBuff, min_x+i, min_y+j, image->pixels[image->width*j+i]);
		}
	}
}

void DrawImageExceptColor(PlatformBitBuffer *bitBuff, SimpleImage *image, int min_x, int min_y, int vanish_color)
{
	for (int j = 0; j < image->height; j++)
	{
		for (int i = 0; i < image->width; i++)
		{
			if ((image->pixels[image->width*j+i] & vanish_color) != vanish_color)
				PlatformDrawPixel(bitBuff, min_x+i, min_y+j, image->pixels[image->width*j+i]);
		}
	}
}

void DrawImageOnlyColor(PlatformBitBuffer *bitBuff, SimpleImage *image, int min_x, int min_y, int single_color)
{
	for (int j = 0; j < image->height; j++)
	{
		for (int i = 0; i < image->width; i++)
		{
			if (image->pixels[image->width*j+i] == single_color)
				PlatformDrawPixel(bitBuff, min_x+i, min_y+j, single_color);
		}
	}
}

int CharToBMPFontCharIndex(char c)
{
	if (c >= 'A' && c <= 'Z')
	{
		return c - 'A';
	}
	if (c >= 'a' && c <= 'z')
	{
		return c - 'a';
	}
	if (c >= '0' && c <= '9')
	{
		return c - '0' + 26;
	}
	if (c == ',' || c == '.')
	{
		return 37;
	}
	if (c == ':')
	{
		return 38;
	}
	if (c == '-')
	{
		return 39;
	}
	if (c == ' ')
	{
		return 36;
	}
	return -1;
}

void DrawBMPFontChar(PlatformBitBuffer *bitBuff, SimpleImage *font, int min_x, int min_y, char c)
{
	int font_color = MakeColor(255, 0, 0, 0);
	int index = CharToBMPFontCharIndex(c);
	if (index < 0)
	{
		return;
	}
	for (int j = 0; j < font->height; j++)
	{
		for (int i = 0; i < 16; i++)
		{
			if (font->pixels[font->width*j+i+index*16] == font_color)
				PlatformDrawPixel(bitBuff, min_x+i, min_y+j, font_color);
		}
	}
}

void DrawBMPFontString(PlatformBitBuffer *bitBuff, SimpleImage *font, int min_x, int min_y, std::string &str)
{
	for (int i = 0; i < str.length(); i++)
	{
		DrawBMPFontChar(bitBuff, font, min_x + i*16, min_y, str[i]);
	}
}
