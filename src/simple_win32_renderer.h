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
#include <Windowsx.h>
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

typedef struct
{
	HWND main_window;
	HDC hdc;
} W32Extra;

typedef int (*CallbackUpdateFunction)(SharedState*);

int ResizePlatformBitBuffer(PlatformBitBuffer *p, int screenWidth, int screenHeight)
{
	p->width = screenWidth;
	p->height = screenHeight;
	
	if (p->bits != 0)
	{
		delete [] p->bits;
	}
	p->bits = new int[screenWidth * screenHeight]{};
	
	BITMAPINFO *info = (BITMAPINFO*)(&p->info);
	info->bmiHeader.biSize = sizeof(info->bmiHeader);
	info->bmiHeader.biWidth = screenWidth;
	info->bmiHeader.biHeight = -screenHeight;
	info->bmiHeader.biPlanes = 1;
	info->bmiHeader.biBitCount = 8 * sizeof(int);
	info->bmiHeader.biCompression = BI_RGB;
	
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
int PlatformUpdateDisplay(SharedState* state, int screenWidth, int screenHeight)
{
	PlatformBitBuffer *bitBuff = state->bitBuff;
	W32Extra *extra = (W32Extra*)(state->extra);
	HDC hdc = extra->hdc;
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
	  (BITMAPINFO*)(&bitBuff->info),
	  DIB_RGB_COLORS,
	  SRCCOPY
	);
	
	return res;
}


int Win32GoBorderlessFullscreen(HWND hwnd)
{
	int w = GetSystemMetrics(SM_CXSCREEN);
	int h = GetSystemMetrics(SM_CYSCREEN);
	SetWindowLongPtr(hwnd, GWL_STYLE, WS_VISIBLE | WS_POPUP);
	SetWindowPos(hwnd, HWND_TOP, 0, 0, w, h, SWP_FRAMECHANGED);
	return 0;
}
int PlatformGoBorderlessFullscreen(SharedState *s)
{
	HWND hwnd = ((W32Extra*)(s->extra))->main_window;
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
		
	case WM_CLOSE:
	{
		shared_state->is_running = 0;
    } break;
	
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		{
			int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
			
			
			
			if (shared_state != 0)
			{
				if (wParam == MK_LBUTTON)
				{
					shared_state->is_lmb_down = 1;
				} else {
					shared_state->is_lmb_down = 0;
				}
				shared_state->mouse_x = x;
				shared_state->mouse_y = y;
			}
		}
		break;
		
	case WM_KEYDOWN:
		{
			if (shared_state != 0)
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
		}
		break;
		
	case WM_SIZE:
		{
			UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
			
			if (shared_state != 0)
			{
				PlatformBitBuffer *bitBuff = shared_state->bitBuff;
				
				shared_state->client_width = width;
				shared_state->client_height = height;
				
				ResizePlatformBitBuffer(bitBuff, width/shared_state->scale, height/shared_state->scale);
				CallbackUpdateFunction UpdateFunc = (CallbackUpdateFunction)(shared_state->callback_update_func);
				if (UpdateFunc == 0)
				{
					FillPlatformBitBuffer(shared_state->bitBuff, MakeColor(255, 255, 255, 255));
				} else {
					UpdateFunc(shared_state);
				}
				HDC hdc = GetDC(hwnd);
				((W32Extra*)(shared_state->extra))->hdc = hdc;
				PlatformUpdateDisplay(shared_state, width, height);
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
				PlatformBitBuffer *bitBuff = shared_state->bitBuff;
				
				shared_state->client_width = width;
				shared_state->client_height = height;
				
				ResizePlatformBitBuffer(bitBuff, width/shared_state->scale, height/shared_state->scale);
				CallbackUpdateFunction UpdateFunc = (CallbackUpdateFunction)(shared_state->callback_update_func);
				if (UpdateFunc == 0)
				{
					FillPlatformBitBuffer(shared_state->bitBuff, MakeColor(255, 255, 255, 255));
				} else {
					UpdateFunc(shared_state);
				}
				((W32Extra*)(shared_state->extra))->hdc = hdc;
				PlatformUpdateDisplay(shared_state, width, height);
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



int InitSharedState(SharedState *shared_state)
{
	int screenWidth = 700;
	int screenHeight = 700;
	
	shared_state->scale = 1;
	shared_state->client_width  = screenWidth  * shared_state->scale;
	shared_state->client_height = screenHeight * shared_state->scale;
	shared_state->dir = 'l';
	
	shared_state->bitBuff = new PlatformBitBuffer{};
	ResizePlatformBitBuffer(shared_state->bitBuff, screenWidth, screenHeight);
	
	shared_state->extra = new W32Extra{};
	shared_state->is_running = 1;
	return 0;
	// TODO: implement uninitializer TerminateSharedState
}
