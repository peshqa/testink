/*
platform_simple_renderer.h - (platform independent) core of all smaller projects that draws things on the screen
2023/09/18, peshqa
*/
#pragma once

#include "simple_3d_gfx.h"

#include <vector>
#include <chrono>
#include <assert.h>
#include <string>

enum InputCode : unsigned int
{
	INPUT_LEFT = 0x25,
	INPUT_UP = 0x26,
	INPUT_RIGHT = 0x27,
	INPUT_DOWN = 0x28,
	INPUT_LSHIFT = 0xA0,
};

enum ARGB8888Color : unsigned int
{
	COLOR_WHITE = 0xFFFFFFFF,
	COLOR_BLACK = 0xFF000000,
	COLOR_RED 	= 0xFFFF0000,
	COLOR_GREEN = 0xFF00FF00,
	COLOR_BLUE 	= 0xFF0000FF
};

enum ScreenMode : int
{
	SCREEN_MODE_WINDOWED = 0,
	SCREEN_MODE_FULLSCREEN = 1,
	SCREEN_MODE_BORDERLESS_FULLSCREEN = 2,
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
	//std::vector<SimpleImage*> images;
	char dir;
	//Project *project;
	void *project_state;
	std::chrono::steady_clock::time_point curr_time;
	std::chrono::steady_clock::time_point prev_time;
	
	std::string asset_path;
	
	int is_lmb_down;
	int mouse_x;
	int mouse_y;
	
	unsigned char input_state[300];
	
	int is_running;
	
	int screen_mode;
	
	unsigned char is_accelerometer_active;
	unsigned char is_gyroscope_active;
	float rot_vec_values[4];
	
	void *extra;
};

// Functions to be implemented by the OS abstraction layer

int PlatformGoBorderlessFullscreen(SharedState *s);
int MakeColor(int a, int r, int g, int b);

int InitAssetManager(SharedState *s);
int OpenAssetFileA(SharedState *s, std::string &filename);
int ReadAssetLineA(SharedState *s, std::string &line);
int ReadAssetBytesA(SharedState *s, char *buf, unsigned int bytes);
int ReadAssetUntilSpaceA(SharedState *s, std::string &line);
//int ReadAssetSkipA(SharedState *s);
int CloseAssetFile(SharedState *s);
int TerminateAssetManager(SharedState *s);

#include "ppm_image_loader.h"

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
int DrawPixelf(PlatformBitBuffer *bitBuff, float x1, float y1, int color)
{
	int end_x = bitBuff->width;
	int end_y = bitBuff->height;
	return PlatformDrawPixel(bitBuff,
			ConvertRelToPlain(x1, 0, end_x),
			ConvertRelToPlain(y1, 0, end_y),
			color);
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
/*
int WideLoadFileOBJ(const wchar_t *filename, std::vector<float*> &points, std::vector<int*> &triangles)
{
	std::wifstream file_obj(filename);
	
	if (file_obj.fail())
	{
		assert(!"failed to open obj file");
		return 1;
	}
	
	std::wstring line;
	
	while(getline(file_obj, line))
	{
		if (line[0] == L'v' && line[1] == L' ')
		{
			// Vertex
			
			std::wstring vals_str = line.substr(2);
			float *vals = new float[3]{};

			std::wstring val0 = vals_str.substr(0, vals_str.find(L' '));
			vals[0] = (float)_wtof(val0.c_str());

			std::wstring val1 = vals_str.substr(val0.length() + 1, vals_str.find(L' ', val0.length() + 1));
			vals[1] = (float)_wtof(val1.c_str());
			
			std::wstring val2 = vals_str.substr(vals_str.find_last_of(L' ') + 1);
			vals[2] = (float)_wtof(val2.c_str());
			
			points.push_back(vals);
		} else if (line[0] == L'v' && line[1] == L't' && line[2] == L' ') {
			// do nothing for now
		} else if (line[0] == L'v' && line[1] == L'n' && line[2] == L' ') {
			// do nothing
		} else if (line[0] == L'f' && line[1] == L' ') {
			// Face
			
			std::wstring lineVals = line.substr(2);

			std::wstring val0 = lineVals.substr(0, lineVals.find_first_of(L' '));

			// If the value for this face includes texture and/or 
			// normal, parse them out
			if (lineVals.find(L'/') >= 0)
			{
				// Get first group of values
				std::wstring g1 = lineVals.substr(0, lineVals.find(' '));
				
				// Get second group of values
				std::wstring g2 = line.substr(line.find(L' ') + 2);
				g2 = g2.substr(g2.find(L' ') + 1);
				g2 = g2.substr(0, g2.find(L' '));
	
				std::wstring g3 = line.substr(line.find_last_of(L' ') + 1);
	
	
				g1 = g1.substr(0, g1.find(L'/'));
				g2 = g2.substr(0, g2.find(L'/'));
				g3 = g3.substr(0, g3.find(L'/'));
				int *vals = new int[3]{};
				vals[0] = (int)_wtoi(g1.c_str()) - 1;
				vals[1] = (int)_wtoi(g2.c_str()) - 1;
				vals[2] = (int)_wtoi(g3.c_str()) - 1;
				triangles.push_back(vals);
			} else {
				assert(!"unhandled case in LoadFileOBJ - face no textures");
			}
		}
	}
	
	return 0;
}*/

int LoadFileOBJ(SharedState *s, std::string &filename, std::vector<float*> &points, std::vector<int*> &triangles)
{
	
	//std::ifstream file_obj(filename);
	
	if (/*file_obj.fail()*/OpenAssetFileA(s, filename) != 0)
	{
		assert(!"failed to open obj file");
		return 1;
	}
	
	std::string line;
	
	while(/*getline(file_obj, line)*/ReadAssetLineA(s, line))
	{
		if (line[0] == 'v' && line[1] == ' ')
		{
			// Vertex
			
			std::string vals_str = line.substr(2);
			float *vals = new float[3]{};

			std::string val0 = vals_str.substr(0, vals_str.find(' '));
			vals[0] = (float)atof(val0.c_str());

			std::string val1 = vals_str.substr(val0.length() + 1, vals_str.find(' ', val0.length() + 1));
			vals[1] = (float)atof(val1.c_str());
			
			std::string val2 = vals_str.substr(vals_str.find_last_of(' ') + 1);
			vals[2] = (float)atof(val2.c_str());
			
			points.push_back(vals);
		} else if (line[0] == 'v' && line[1] == 't' && line[2] == ' ') {
			// do nothing for now
		} else if (line[0] == 'v' && line[1] == 'n' && line[2] == ' ') {
			// do nothing
		} else if (line[0] == 'f' && line[1] == ' ') {
			// Face
			
			std::string lineVals = line.substr(2);

			std::string val0 = lineVals.substr(0, lineVals.find_first_of(' '));

			// If the value for this face includes texture and/or 
			// normal, parse them out
			if (lineVals.find('/') >= 0)
			{
				// Get first group of values
				std::string g1 = lineVals.substr(0, lineVals.find(' '));
				
				// Get second group of values
				std::string g2 = line.substr(line.find(' ') + 2);
				g2 = g2.substr(g2.find(' ') + 1);
				g2 = g2.substr(0, g2.find(' '));
	
				std::string g3 = line.substr(line.find_last_of(' ') + 1);
	
	
				g1 = g1.substr(0, g1.find('/'));
				g2 = g2.substr(0, g2.find('/'));
				g3 = g3.substr(0, g3.find('/'));
				int *vals = new int[3]{};
				vals[0] = (int)atoi(g1.c_str()) - 1;
				vals[1] = (int)atoi(g2.c_str()) - 1;
				vals[2] = (int)atoi(g3.c_str()) - 1;
				triangles.push_back(vals);
			} else {
				assert(!"unhandled case in LoadFileOBJ - face no textures");
			}
		}
	}
	
	CloseAssetFile(s);
	
	return 0;
}
