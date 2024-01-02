/*
ppm_image_loader.h - this header file combines different file format loaders
2023/12/28, peshqa
*/
#pragma once

// define some common stuff
typedef struct
{
	int width;
	int height;
	// assuming its 4 bytes per pixel
	int *pixels;
} SimpleImage;

#include "ppm_image_loader.h"
#include "bmp_image_loader.h"

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
