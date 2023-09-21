/*
platform_simple_renderer.h - (platform independent) core of all smaller projects that draws things on the screen
2023/09/18, peshqa
*/
#pragma once

#include "ppm_image_loader.h"

#include <vector>
#include <chrono>

enum ARGB8888Color : unsigned int
{
	COLOR_WHITE = 0xFFFFFFFF,
	COLOR_BLACK = 0xFF000000,
	COLOR_RED 	= 0xFFFF0000,
	COLOR_GREEN = 0xFF00FF00,
	COLOR_BLUE 	= 0xFF0000FF
};

struct PlatformBitBuffer
{
	int width;
	int height;
	void *bits;
	void* info;
};
struct SharedState
{
	PlatformBitBuffer* bitBuff;
	void *callback_update_func;
	int client_width;
	int client_height;
	int scale;
	std::vector<SimpleImage*> images;
	char dir;
	//Project *project;
	void *project_state;
	std::chrono::steady_clock::time_point curr_time;
	std::chrono::steady_clock::time_point prev_time;
};

int MakeColor(int a, int r, int g, int b);

float CalculateDeltaTime(SharedState* state)
{
	state->prev_time = state->curr_time;
	state->curr_time = std::chrono::steady_clock::now();
	std::chrono::duration<float> dur = state->curr_time - state->prev_time;
	float elapsedTime = dur.count();
	return elapsedTime;
}
float GetDeltaTime(SharedState* state)
{
	std::chrono::duration<float> dur = state->curr_time - state->prev_time;
	float elapsedTime = dur.count();
	return elapsedTime;
}

int PlatformDrawPixel(PlatformBitBuffer *bitBuff, int x, int y, int color);

int ResizePlatformBitBuffer(PlatformBitBuffer *p, int screenWidth, int screenHeight);

int GrayscalePlatformBitBuffer(PlatformBitBuffer *bitBuff);
int RedPlatformBitBuffer(PlatformBitBuffer *bitBuff);

int PlatformDrawLine(PlatformBitBuffer *bitBuff, int x1, int y1, int x2, int y2, int color)
{
	// naive algorithm
	/*int tmp{};
	if (x2 <= x1)
	{
		tmp = x1;
		x1 = x2;
		x2 = tmp;
		
		tmp = y1;
		y1 = y2;
		y2 = tmp;
	}
	
	int dx = x2 - x1;
	int dy = y2 - y1;
	
	if (dx == 0)
	{
		return 0;
	}
	
	for (int x = x1; x <= x2; x++)
	{
		int y = y1 + dy * (x - x1) / dx;
		Win32DrawPixel(bitBuff, x, y);
	}*/
	
	// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
	// TODO: add edge cases for horizontal and vertical lines
	int dx = abs(x2 - x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1);
    int sy = y1 < y2 ? 1 : -1;
    int error = dx + dy;
    
    while (true)
	{
        PlatformDrawPixel(bitBuff, x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * error;
        if (e2 >= dy)
		{
            if (x1 == x2) break;
            error = error + dy;
            x1 = x1 + sx;
        }
        if (e2 <= dx)
		{
            if (y1 == y2) break;
            error = error + dx;
            y1 = y1 + sy;
        }
    }
	
	return 0;
}

int PlatformFillRect(PlatformBitBuffer *bitBuff, int left, int top, int right, int bottom, int color)
{
	for (int y = top; y < bottom; y++)
	{
		for (int x = left; x < right; x++)
		{
			PlatformDrawPixel(bitBuff, x, y, color);
		}
	}
	return 0;
}

int GetPlatformBitBufferWidth(PlatformBitBuffer *bitBuff)
{
	return bitBuff->width;
}
int GetPlatformBitBufferHeight(PlatformBitBuffer *bitBuff)
{
	return bitBuff->height;
}

int FillPlatformBitBuffer(PlatformBitBuffer *bitBuff, int color)
{
	return PlatformFillRect(bitBuff, 0, 0,
		GetPlatformBitBufferWidth(bitBuff),
		GetPlatformBitBufferHeight(bitBuff), color);
}

int ConvertRelXToX(float rel_x, PlatformBitBuffer *bitBuff)
{
	return (int)(bitBuff->width)*rel_x;
}
int ConvertRelYToY(float rel_y, PlatformBitBuffer *bitBuff)
{
	return (int)(bitBuff->height)*rel_y;
}
int ConvertRelXToXse(float rel_x, int start, int end)
{
	int length = end - start;
	return (int)(length)*rel_x + start;
}
int ConvertRelYToYse(float rel_y, int start, int end)
{
	int length = end - start;
	return (int)(length)*rel_y + start;
}
