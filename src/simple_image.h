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
