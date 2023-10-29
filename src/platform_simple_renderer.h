/*
platform_simple_renderer.h - (platform independent) core of all smaller projects that draws things on the screen
2023/09/18, peshqa
*/
#pragma once

#include "simple_3d_gfx.h"
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
	void *info;
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
	
	int is_lmb_down;
	int mouse_x;
	int mouse_y;
	
	int is_running;
	
	void *extra;
};

int PlatformGoBorderlessFullscreen(SharedState *s);
int MakeColor(int a, int r, int g, int b);

int ConvertRelToPlain(float rel, int start, int end)
{
	int length = end - start;
	return (int)(length)*rel + start;
}

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
int PlatformDrawLinef(PlatformBitBuffer *bitBuff, float x1, float y1, float x2, float y2, int color)
{
	int end_x = bitBuff->width;
	int end_y = bitBuff->height;
	return PlatformDrawLine(bitBuff,
			ConvertRelToPlain(x1, 0, end_x),
			ConvertRelToPlain(y1, 0, end_y),
			ConvertRelToPlain(x2, 0, end_x),
			ConvertRelToPlain(y2, 0, end_y),
			color);
}
// util function to draw filled triangle
int PlatformGetLineXValues(int x1, int y1, int x2, int y2, std::vector<int> &v)
{
	int dx = abs(x2 - x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1);
    int sy = y1 < y2 ? 1 : -1;
    int error = dx + dy;
	
	int last_x = x1-1;
	int last_y = y1;
    
    while (true)
	{
        //PlatformDrawPixel(bitBuff, x1, y1, color);
		if (last_y != y1)
		{
			v.push_back(x1);
		}
		last_y = y1;
		last_x = x1;
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
	v.push_back(last_x);
	return 0;
}
int DrawHorizontalLine(PlatformBitBuffer *bitBuff, int x1, int x2, int y, int color)
{
	int tmp;
	if (x2 < x1)
	{
		tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	for (int i = x1; i <= x2; i++)
	{
		PlatformDrawPixel(bitBuff, i, y, color);
	}
	return 0;
}
int DrawTrianglef(PlatformBitBuffer *bitBuff, float x1, float y1, float x2, float y2, float x3, float y3, int color)
{
	PlatformDrawLinef(bitBuff, x1, y1, x2, y2, color);
	PlatformDrawLinef(bitBuff, x1, y1, x3, y3, color);
	PlatformDrawLinef(bitBuff, x2, y2, x3, y3, color);
	return 0;
}
int FillTriangle(PlatformBitBuffer *bitBuff, int x1, int y1, int x2, int y2, int x3, int y3, int color)
{
	// Sort the points so that y1 <= y2 <= y3
	int tmp;
	if (y2 < y1)
	{
		tmp = y1;
		y1 = y2;
		y2 = tmp;
		tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	if (y3 < y1)
	{
		tmp = y1;
		y1 = y3;
		y3 = tmp;
		tmp = x1;
		x1 = x3;
		x3 = tmp;
	}
	if (y3 < y2)
	{
		tmp = y2;
		y2 = y3;
		y3 = tmp;
		tmp = x2;
		x2 = x3;
		x3 = tmp;
	}
	
	// Compute the x coordinates of the triangle edges
	std::vector<int> v1;
	std::vector<int> v2;
	PlatformGetLineXValues(x1, y1, x2, y2, v1);
	v1.pop_back();
	PlatformGetLineXValues(x2, y2, x3, y3, v1);
	PlatformGetLineXValues(x1, y1, x3, y3, v2);
	
	int idx = 0;
	for (int i = y1; i <= y3; i++)
	{
		DrawHorizontalLine(bitBuff, v1[idx], v2[idx], i, color);
		idx++;
	}
	
	return 0;
}
int FillTrianglef(PlatformBitBuffer *bitBuff, float x1, float y1, float x2, float y2, float x3, float y3, int color)
{
	int end_x = bitBuff->width;
	int end_y = bitBuff->height;
	return FillTriangle(bitBuff,
			ConvertRelToPlain(x1, 0, end_x),
			ConvertRelToPlain(y1, 0, end_y),
			ConvertRelToPlain(x2, 0, end_x),
			ConvertRelToPlain(y2, 0, end_y),
			ConvertRelToPlain(x3, 0, end_x),
			ConvertRelToPlain(y3, 0, end_y),
			color);
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
// TODO: get rid of those two below
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
