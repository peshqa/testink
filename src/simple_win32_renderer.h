/*
simple_win32_renderer.h - core of all smaller projects that draws things on the screen
2023/08/09, peshqa
*/
#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <chrono>
#include <string>
#include <vector>

#include "ppm_image_loader.h"
//#include "project.h"

#pragma comment (lib, "Gdi32.lib")
#pragma comment (lib, "User32.lib")

enum RGBColor : int
{
	COLOR_WHITE = 0x00FFFFFF,
	COLOR_BLACK = 0x00000000,
	COLOR_RED 	= 0x00FF0000,
	COLOR_GREEN = 0x0000FF00,
	COLOR_BLUE 	= 0x000000FF
};

typedef struct
{
	int width;
	int height;
	void *bits;
	BITMAPINFO info;
} W32BitBuffer;

typedef struct
{
	W32BitBuffer* bitBuff;
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
} SharedState;

typedef int (*CallbackUpdateFunction)(SharedState*);

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

int ResizeW32BitBuffer(W32BitBuffer *p, int screenWidth, int screenHeight)
{
	p->width = screenWidth;
	p->height = screenHeight;
	
	if (p->bits != 0)
	{
		delete [] p->bits;
	}
	p->bits = new int[screenWidth * screenHeight]{};
	
	p->info.bmiHeader.biSize = sizeof(p->info.bmiHeader);
	p->info.bmiHeader.biWidth = screenWidth;
	p->info.bmiHeader.biHeight = -screenHeight;
	p->info.bmiHeader.biPlanes = 1;
	p->info.bmiHeader.biBitCount = 8 * sizeof(int);
	p->info.bmiHeader.biCompression = BI_RGB;
	
	return 0;
}

int Win32DrawPixel(W32BitBuffer *bitBuff, int x, int y, int xxrrggbb=COLOR_BLACK)
{
	if (x < 0 || x >= bitBuff->width || y < 0 || y >= bitBuff->height)
	{
		return -1;
	}
	((int*)(bitBuff->bits))[y*bitBuff->width+x] = xxrrggbb;
	return 0;
}

int FillW32BitBuffer(W32BitBuffer *bitBuff, int color)
{
	for (int i = 0; i < bitBuff->width*bitBuff->height; i++)
	{
		((int*)bitBuff->bits)[i] = color;
	}
	
	return 0;
}

int Win32DrawLine(W32BitBuffer *bitBuff, int x1, int y1, int x2, int y2)
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
        Win32DrawPixel(bitBuff, x1, y1);
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

int Win32FillRect(W32BitBuffer *bitBuff, int left, int top, int right, int bottom, int color)
{
	for (int y = top; y < bottom; y++)
	{
		for (int x = left; x < right; x++)
		{
			Win32DrawPixel(bitBuff, x, y, color);
		}
	}
	return 0;
}

int W32UpdateDisplay(HDC hdc, int screenWidth, int screenHeight, W32BitBuffer *bitBuff)
{
	int res = StretchDIBits(
	  hdc,
	  0,
	  0,
	  screenWidth,
	  screenHeight,
	  0,
	  0,
	  bitBuff->width,
	  bitBuff->height,
	  bitBuff->bits,
	  &bitBuff->info,
	  DIB_RGB_COLORS,
	  SRCCOPY
	);
	
	return res;
}

int ConvertRelXToX(float rel_x, W32BitBuffer *bitBuff)
{
	return (int)(bitBuff->width)*rel_x;
}
int ConvertRelYToY(float rel_y, W32BitBuffer *bitBuff)
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

int PaintW32BitBuffer(W32BitBuffer *bitBuff)
{
	FillW32BitBuffer(bitBuff, COLOR_WHITE);
	
	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT res = 0;
	
    switch (uMsg)
    {
    case WM_DESTROY:
		{
			PostQuitMessage(0);
		}
		break;
		
	case WM_LBUTTONDOWN:
	//case WM_MOUSEMOVE:
		{
			int x = LOWORD(lParam);
            int y = HIWORD(lParam);
			
			SharedState *shared_state{};
			shared_state = (SharedState*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			
			if (shared_state != 0)
			{
				W32BitBuffer *bitBuff = shared_state->bitBuff;
				//Win32DrawLine(bitBuff, 200/shared_state->scale, 200/shared_state->scale, x/shared_state->scale, y/shared_state->scale);
				Win32DrawPixel(bitBuff, x/shared_state->scale, y/shared_state->scale);
			}
		}
		break;
		
	case WM_RBUTTONDOWN:
		{
			int x = LOWORD(lParam);
            int y = HIWORD(lParam);
			
			SharedState *shared_state{};
			shared_state = (SharedState*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			
			if (shared_state != 0)
			{
				W32BitBuffer *bitBuff = shared_state->bitBuff;
				//Win32DrawLine(bitBuff, 200/shared_state->scale, 200/shared_state->scale, x/shared_state->scale, y/shared_state->scale);
				Win32DrawPixel(bitBuff, x/shared_state->scale, y/shared_state->scale, RGB(255,255,255));
			}
		}
		break;
		
	case WM_KEYDOWN:
		{
			SharedState *shared_state{};
			shared_state = (SharedState*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			
			if (wParam == VK_LEFT)
			{
				shared_state->dir = 'l';
			} else if (wParam == VK_UP)
			{
				shared_state->dir = 'u';
			} else if (wParam == VK_RIGHT)
			{
				shared_state->dir = 'r';
			} else if (wParam == VK_DOWN)
			{
				shared_state->dir = 'd';
			}
		}
		break;
		
	case WM_SIZE:
		{
			SharedState *shared_state{};
			shared_state = (SharedState*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			
			UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
			
			if (shared_state != 0)
			{
				W32BitBuffer *bitBuff = shared_state->bitBuff;
				
				shared_state->client_width = width;
				shared_state->client_height = height;
				
				ResizeW32BitBuffer(bitBuff, width/shared_state->scale, height/shared_state->scale);
				CallbackUpdateFunction UpdateFunc = (CallbackUpdateFunction)(shared_state->callback_update_func);
				if (UpdateFunc == 0)
				{
					PaintW32BitBuffer(bitBuff);
				} else {
					UpdateFunc(shared_state);
				}
				HDC hdc = GetDC(hwnd);
				W32UpdateDisplay(hdc, width, height, bitBuff);
				ReleaseDC(hwnd, hdc);
			}
		}
		break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
			
			RECT clientRect{};
			GetClientRect(hwnd, &clientRect);
			
			SharedState *shared_state{};
			shared_state = (SharedState*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			
			UINT width = clientRect.right - clientRect.left;
            UINT height = clientRect.bottom - clientRect.top;
			
			if (shared_state != 0)
			{
				W32BitBuffer *bitBuff = shared_state->bitBuff;
				
				shared_state->client_width = width;
				shared_state->client_height = height;
				
				ResizeW32BitBuffer(bitBuff, width/shared_state->scale, height/shared_state->scale);
				CallbackUpdateFunction UpdateFunc = (CallbackUpdateFunction)(shared_state->callback_update_func);
				if (UpdateFunc == 0)
				{
					PaintW32BitBuffer(bitBuff);
				} else {
					UpdateFunc(shared_state);
				}
				W32UpdateDisplay(hdc, width, height, bitBuff);
				ReleaseDC(hwnd, hdc);
			}
			EndPaint(hwnd, &ps);
        }
        break;

	default:
		{
			// TODO: there is an issue with windows' moving and sizing 'modal mode'
			// which causes the app to freeze while user holds LMB on one of the window borders
			// https://stackoverflow.com/questions/3102074/win32-my-application-freezes-while-the-user-resizes-the-window
			res = DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
		break;
    }
	
	return res;
}