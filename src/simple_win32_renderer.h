/*
simple_win32_renderer.h - (Windows specific) core of all smaller projects that draws things on the screen
2023/08/09, peshqa
*/
#pragma once

#include "platform_simple_renderer.h"

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <string>
#include <vector>

//#include "project.h"

#pragma comment (lib, "Gdi32.lib")
#pragma comment (lib, "User32.lib")

typedef struct
{
	int width;
	int height;
	void *bits;
	BITMAPINFO info;
} W32BitBuffer;

typedef int (*CallbackUpdateFunction)(SharedState*);

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
int PlatformDrawPixel(PlatformBitBuffer *bB, int x, int y, int color)
{
	W32BitBuffer *bitBuff = (W32BitBuffer*)bB;
	if (x < 0 || x >= bitBuff->width || y < 0 || y >= bitBuff->height)
	{
		return -1;
	}
	((int*)(bitBuff->bits))[y*bitBuff->width+x] = color;
	return 0;
}

int MakeColor(int a, int r, int g, int b)
{
	return (a<<24) + (r<<16) + (g<<8) + b;
}

int FillW32BitBuffer(W32BitBuffer *bitBuff, int color)
{
	for (int i = 0; i < bitBuff->width*bitBuff->height; i++)
	{
		((int*)bitBuff->bits)[i] = color;
	}
	
	return 0;
}
/*int FillPlatformBitBuffer(PlatformBitBuffer *bitBuff, int color)
{
	return FillW32BitBuffer((W32BitBuffer*)bitBuff, color);
}*/

int GrayscaleW32BitBuffer(W32BitBuffer *bitBuff)
{
	for (int i = 0; i < bitBuff->width*bitBuff->height; i++)
	{
		int color = ((int*)(bitBuff->bits))[i];
		color = (((color & 0x00FF0000) >> 16) + ((color & 0x0000FF00) >> 8) + (color & 0x000000FF)) / 3;
		((int*)bitBuff->bits)[i] = color + (color << 8) + (color << 16);
	}
	
	return 0;
}
int RedW32BitBuffer(W32BitBuffer *bitBuff)
{
	for (int i = 0; i < bitBuff->width*bitBuff->height; i++)
	{
		int color = ((int*)(bitBuff->bits))[i];
		((int*)bitBuff->bits)[i] = color & 0x00FF0000;
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

int PaintW32BitBuffer(W32BitBuffer *bitBuff)
{
	FillW32BitBuffer(bitBuff, COLOR_WHITE);
	
	return 0;
}

int Win32GoBorderlessFullscreen(HWND hwnd)
{
	int w = GetSystemMetrics(SM_CXSCREEN);
	int h = GetSystemMetrics(SM_CYSCREEN);
	SetWindowLongPtr(hwnd, GWL_STYLE, WS_VISIBLE | WS_POPUP);
	SetWindowPos(hwnd, HWND_TOP, 0, 0, w, h, SWP_FRAMECHANGED);
	return 0;
}
int Win32GoFullscreen(HWND hwnd)
{
	// TODO: figure out how to implement fullscreen mode
	// TODO: read this https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
	/*DEVMODE dev{};
	dev.dmBitsPerPel = 32;
	dmPelsWidth = 1;
	dmPelsHeight = 1;
	dmDisplayFlags = DM_BITSPERPEL;
	ChangeDisplaySettings(0, CDS_FULLSCREEN);*/
	return 1;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT res = 0;
	SharedState *shared_state{};
	shared_state = (SharedState*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	
    switch (uMsg)
    {
    case WM_DESTROY:
		{
			PostQuitMessage(0);
		}
		break;
		
	case WM_LBUTTONDOWN:
	//case WM_MOUSEMOVE:
	//case WM_RBUTTONDOWN:
		{
			int x = LOWORD(lParam);
            int y = HIWORD(lParam);
			
			if (shared_state != 0)
			{
			}
		}
		break;
		
	case WM_KEYDOWN:
		{
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
			UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
			
			if (shared_state != 0)
			{
				W32BitBuffer *bitBuff = (W32BitBuffer*)shared_state->bitBuff;
				
				shared_state->client_width = width;
				shared_state->client_height = height;
				
				ResizeW32BitBuffer(bitBuff, width/shared_state->scale, height/shared_state->scale);
				CallbackUpdateFunction UpdateFunc = (CallbackUpdateFunction)(shared_state->callback_update_func);
				if (UpdateFunc == 0)
				{
					PaintW32BitBuffer(bitBuff);
				} else {
					UpdateFunc((SharedState*)shared_state);
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
			
			UINT width = clientRect.right - clientRect.left;
            UINT height = clientRect.bottom - clientRect.top;
			
			if (shared_state != 0)
			{
				W32BitBuffer *bitBuff = (W32BitBuffer*)shared_state->bitBuff;
				
				shared_state->client_width = width;
				shared_state->client_height = height;
				
				ResizeW32BitBuffer(bitBuff, width/shared_state->scale, height/shared_state->scale);
				CallbackUpdateFunction UpdateFunc = (CallbackUpdateFunction)(shared_state->callback_update_func);
				if (UpdateFunc == 0)
				{
					PaintW32BitBuffer(bitBuff);
				} else {
					UpdateFunc((SharedState*)shared_state);
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
