#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <chrono>
#include <string>

#include "simple_qr.h"

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
	W32BitBuffer* bitBuff;
	int client_width;
	int client_height;
	int scale;
} SharedState;

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

int DrawQR(W32BitBuffer *bitBuff, QRCode *qr)
{
	int qr_size = VersionToSize(qr->version);
	for (int y = 0; y < qr_size; y++)
	{
		for (int x = 0; x < qr_size; x++)
		{
			if (qr->code_bytes[y*qr_size+x] == QR_DARK)
			{
				((int*)bitBuff->bits)[y*bitBuff->width+x] = 0x00000000;
			}
			else if (qr->code_bytes[y*qr_size+x] == QR_UNASSIGNED)
			{
				((int*)bitBuff->bits)[y*bitBuff->width+x] = 0x00777777;
			}
			else if (qr->code_bytes[y*qr_size+x] == QR_RESERVED)
			{
				((int*)bitBuff->bits)[y*bitBuff->width+x] = 0x003333FF;
			}
		}
	}
	return 0;
}

int PaintW32BitBuffer(W32BitBuffer *bitBuff)
{
	for (int i = 0; i < bitBuff->width*bitBuff->height; i++)
	{
		int r = rand() % 0x100;
		((int*)bitBuff->bits)[i] = rand()%256+(r<<8)+(r<<16);
		//((int*)bitBuff.bits)[i] = (rand()%0x100)+(rand()%0x100<<8)+(rand()%0x100<<16);
		((int*)bitBuff->bits)[i] = 0x00FFFFFF;
	}
	
	QRCode qr{};
	InitQRCode(&qr, 1);
	ApplyCornerFinderPatterns(&qr);
	ApplySeparators(&qr);
	ApplyTimingPatterns(&qr);
	ApplyDarkModule(&qr);
	ApplyAlignmentPatterns(&qr);
	ReserveFormatInformation(&qr);
	ReserveVersionInformation(&qr);
	ApplyDataAndMask(&qr);
	DrawQR(bitBuff, &qr);
	TerminateQRCode(&qr);
	
	return 0;
}

int Win32DrawPixel(W32BitBuffer *bitBuff, int x, int y, int xxrrggbb=0x00000000)
{
	if (x < 0 || x >= bitBuff->width || y < 0 || y >= bitBuff->height)
	{
		return -1;
	}
	((int*)(bitBuff->bits))[y*bitBuff->width+x] = xxrrggbb;
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
				PaintW32BitBuffer(bitBuff);
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
				PaintW32BitBuffer(bitBuff);
				W32UpdateDisplay(hdc, width, height, bitBuff);
				ReleaseDC(hwnd, hdc);
			}
			EndPaint(hwnd, &ps);
        }
        break;

	default:
		{
			res = DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
		break;
    }
	
	return res;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	SharedState shared_state{};
	
	int screenWidth = 100;
	int screenHeight = 100;
	shared_state.scale = 8;
	
	shared_state.client_width = screenWidth*shared_state.scale;
	shared_state.client_height = screenHeight*shared_state.scale;
	
	DWORD window_styles = WS_OVERLAPPEDWINDOW;
	
	RECT window_rect = {0, 0, shared_state.client_width, shared_state.client_height};
	AdjustWindowRect(&window_rect, window_styles, 0);
	
    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"Main Window Class";
    
    WNDCLASS wc = { };

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Learn to Program Windows",    // Window text
        window_styles,            // Window style

        // position and size
        100, 100, window_rect.right, window_rect.bottom,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
        );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

	W32BitBuffer bitBuff{};
	shared_state.bitBuff = &bitBuff;
	ResizeW32BitBuffer(&bitBuff, screenWidth, screenHeight);
	
	// init screen buffer
	PaintW32BitBuffer(&bitBuff);
	
	SetWindowLongPtrW(
		hwnd,
		GWLP_USERDATA,
		(LONG_PTR)&shared_state
	);
	
	std::chrono::steady_clock::time_point currTime = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point prevTime{};
	

    // Run the message loop.
	
	int running = true;
    MSG msg{};
	while (running) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				running = false;
				break;
			}
			
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		prevTime = currTime;
		currTime = std::chrono::steady_clock::now();
		std::chrono::duration<double> dur = currTime - prevTime;
		double elapsedTime = dur.count();
		
		//std::wstring title = std::to_wstring((elapsedTime));
		//SetWindowTextW(hwnd, title.c_str());
		
		HDC hdc = GetDC(hwnd);
		W32UpdateDisplay(hdc, shared_state.client_width, shared_state.client_height, &bitBuff);
		ReleaseDC(hwnd, hdc);
		
		Sleep(50); // ms
	}

    return 0;
}
