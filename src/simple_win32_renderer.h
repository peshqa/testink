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

//#pragma comment (lib, "Gdi32.lib")
//#pragma comment (lib, "User32.lib")

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
	
	std::ifstream file;
} W32Extra;

typedef int (*CallbackUpdateFunction)(SharedState*);

static int ResizePlatformBitBuffer(PlatformBitBuffer *p, int screenWidth, int screenHeight)
{
	p->width = screenWidth;
	p->height = screenHeight;
	p->stride = screenWidth;
	
	if (p->info != 0)
	{
		VirtualFree(p->info, 0, MEM_RELEASE);
	}
	int mem_size = screenWidth * screenHeight * sizeof(int) + sizeof(BITMAPINFO);
	p->info = VirtualAlloc(0, mem_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	p->bits = (char*)p->info + sizeof(BITMAPINFO);

	
	BITMAPINFO *info = (BITMAPINFO*)(p->info);
	info->bmiHeader.biSize = sizeof(info->bmiHeader);
	info->bmiHeader.biWidth = screenWidth;
	info->bmiHeader.biHeight = screenHeight;
	info->bmiHeader.biPlanes = 1;
	info->bmiHeader.biBitCount = 8 * sizeof(int);
	info->bmiHeader.biCompression = BI_RGB;
	
	return 0;
}

static int MakeColor(int a, int r, int g, int b)
{
	return (a<<24) + (r<<16) + (g<<8) + b;
}

static int W32UpdateDisplay(HDC hdc, int screenWidth, int screenHeight, W32BitBuffer *bitBuff)
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
static int PlatformUpdateDisplay(SharedState* state, int screenWidth, int screenHeight)
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
	  (BITMAPINFO*)(bitBuff->info),
	  DIB_RGB_COLORS,
	  SRCCOPY
	);
	
	return res;
}

static int Win32GoBorderlessFullscreen(HWND hwnd)
{
	int w = GetSystemMetrics(SM_CXSCREEN);
	int h = GetSystemMetrics(SM_CYSCREEN);
	SetWindowLongPtr(hwnd, GWL_STYLE, WS_VISIBLE | WS_POPUP);
	SetWindowPos(hwnd, HWND_TOP, 0, 0, w, h, SWP_FRAMECHANGED);
	return 0;
}
static int PlatformGoBorderlessFullscreen(SharedState *s)
{
	HWND hwnd = ((W32Extra*)(s->extra))->main_window;
	int w = GetSystemMetrics(SM_CXSCREEN);
	int h = GetSystemMetrics(SM_CYSCREEN);
	SetWindowLongPtr(hwnd, GWL_STYLE, WS_VISIBLE | WS_POPUP);
	SetWindowPos(hwnd, HWND_TOP, 0, 0, w, h, SWP_FRAMECHANGED);
	return 0;
}
static int PlatformGoWindowed(SharedState *s)
{
	HWND hwnd = ((W32Extra*)(s->extra))->main_window;
	SetWindowLongPtr(hwnd, GWL_STYLE, WS_VISIBLE | WS_OVERLAPPEDWINDOW);
	SetWindowPos(hwnd, HWND_TOP, 100, 100, 500, 500, SWP_FRAMECHANGED);
	return 0;
}
static int Win32GoFullscreen(HWND hwnd)
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

static int ToggleFullscreen(SharedState *shared_state)
{
	if (shared_state->screen_mode == SCREEN_MODE_WINDOWED)
	{
		PlatformGoBorderlessFullscreen(shared_state);
		shared_state->screen_mode = SCREEN_MODE_BORDERLESS_FULLSCREEN;
	} else {
		PlatformGoWindowed(shared_state);
		shared_state->screen_mode = SCREEN_MODE_WINDOWED;
	}
	
	return 0;
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	{
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		
		//bool was_down = (lParam & (1 << 30)) != 0;
		//bool is_down  = (lParam & (1 << 31)) == 0;
		
		if (shared_state != 0)
		{
			shared_state->mouse_x = x;
			shared_state->mouse_y = y;
			
			if (wParam == MK_LBUTTON)
			{
				shared_state->is_lmb_down <<= 1;
				shared_state->is_lmb_down += 1;
			} else {
				shared_state->is_lmb_down <<= 1;
				//shared_state->is_lmb_down = 0;
			}
		}
	}
	
	case WM_KEYUP:
	case WM_KEYDOWN:
	case WM_SYSKEYUP:
    case WM_SYSKEYDOWN:
	{
		WORD vkCode = LOWORD(wParam);
		WORD keyFlags = HIWORD(lParam);
		
		WORD scanCode = LOBYTE(keyFlags);
		BOOL isExtendedKey = (keyFlags & KF_EXTENDED) == KF_EXTENDED;
		
		if (isExtendedKey)
			scanCode = MAKEWORD(scanCode, 0xE0);
		
		bool was_down = (lParam & (1 << 30)) != 0;
		bool is_down  = (lParam & (1 << 31)) == 0;
		
		if (shared_state != 0)
		{
			if (is_down == true)
			{
				bool is_alt_down = lParam & (1 << 29);
				if (wParam == VK_F4 & is_alt_down)
				{
					shared_state->is_running = 0;
				}
				if (wParam == VK_RETURN & is_alt_down)
				{
					ToggleFullscreen(shared_state);
				}
			}
			switch (vkCode)
			{
			case VK_SHIFT:   // converts to VK_LSHIFT or VK_RSHIFT
			case VK_CONTROL: // converts to VK_LCONTROL or VK_RCONTROL
			case VK_MENU:    // converts to VK_LMENU or VK_RMENU
				vkCode = LOWORD(MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX));
			default:
				shared_state->input_state[vkCode] <<= 1;
				shared_state->input_state[vkCode] += is_down;
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
	
	shared_state->screen_mode = SCREEN_MODE_BORDERLESS_FULLSCREEN;
	
	shared_state->is_accelerometer_active = 0;
	shared_state->is_gyroscope_active = 0;
	
	shared_state->asset_path = "../assets/";
	
	shared_state->max_fps = 60;
	
	return 0;
	// TODO: implement uninitializer TerminateSharedState
}


static int OpenAssetFileA(SharedState *s, std::string &filename)
{
	W32Extra *extra = (W32Extra*)(s->extra);
	extra->file.open(filename, std::ifstream::binary);
	return extra->file.fail();
}
static int ReadAssetLineA(SharedState *s, std::string &line)
{
	W32Extra *extra = (W32Extra*)(s->extra);
	return !getline(extra->file, line).eof();
}
static int ReadAssetBytesA(SharedState *s, char *buffer, unsigned int bytes)
{
	W32Extra *extra = (W32Extra*)(s->extra);
	extra->file.read(buffer, bytes);
	return 0;
}
static int ReadAssetUntilSpaceA(SharedState *s, std::string &line)
{
	W32Extra *extra = (W32Extra*)(s->extra);
	extra->file >> line;
	return 0;
}
static int CloseAssetFile(SharedState *s)
{
	W32Extra *extra = (W32Extra*)(s->extra);
	extra->file.close();
	return 0;
}
