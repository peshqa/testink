/*
android_simple_renderer.h - (Android specific) core of all smaller projects that draws things on the screen
2023/09/20, peshqa
*/
#pragma once

#include "platform_simple_renderer.h"

#include <stdlib.h>
#include <string>
#include <vector>

int PlatformDrawPixel(PlatformBitBuffer *bitBuff, int x, int y, int color)
{
	if (x < 0 || x >= bitBuff->width || y < 0 || y >= bitBuff->height)
	{
		return -1;
	}
	((int*)(bitBuff->bits))[y*bitBuff->width+x] = color;
	return 0;
}

int ResizeAndroidBitBuffer(SharedState *state, int screenWidth, int screenHeight)
{
	/*PlatformBitBuffer* p = state->bitBuff;
	p->width = screenWidth;
	p->height = screenHeight;
	
	state->client_width = screenWidth;
	state->client_height = screenHeight;
	
	if (p->bits != 0)
	{
		delete [] (int*)p->bits;
	}
	p->bits = new int[screenWidth * screenHeight]{};*/
	
	return 0;
}

int MakeColor(int a, int r, int g, int b)
{
	return (a<<24) + (b<<16) + (g<<8) + r;
}