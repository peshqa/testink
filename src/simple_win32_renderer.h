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

#include <gl/gl.h>

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

void InitTexture_OpenGL(SimpleImage *img, int index)
{
	glBindTexture(GL_TEXTURE_2D, index);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img->width, img->height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, img->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
}

static int PlatformUpdateDisplay(SharedState* state, int screenWidth, int screenHeight)
{
	PlatformBitBuffer *bitBuff = state->bitBuff;
	W32Extra *extra = (W32Extra*)(state->extra);
	HDC hdc = extra->hdc;
	
#if 0
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
#elif 1
	glViewport(0, 0, screenWidth, screenHeight);
	
	glBindTexture(GL_TEXTURE_2D, 1);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, bitBuff->width, bitBuff->height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, bitBuff->bits);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glEnable(GL_TEXTURE_2D);

	glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glBegin(GL_TRIANGLES);
	
	glColor3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2i(-1, -1);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2i(1, -1);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2i(1, 1);
	
	glTexCoord2f(0.0f, 0.0f);
	glVertex2i(-1, -1);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2i(1, 1);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2i(-1, 1);
	
	glEnd();
	
	glDisable(GL_TEXTURE_2D);
	SwapBuffers(hdc);
#endif
	
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
	
	shared_state->cmdBuff.max_size = MEGABYTES(8);
	shared_state->cmdBuff.base_memory = new u8[shared_state->cmdBuff.max_size];
	shared_state->cmdBuff.used = 0;
	shared_state->cmdBuff.cmd_count = 0;
	
	shared_state->texture_count = 2;
	
	return 0;
	// TODO: implement uninitializer TerminateSharedState. But why?
}

void ProcessCommandBuffer_OpenGL(PlatformBitBuffer *bitBuff, CommandBuffer *cmdBuff)
{
	glViewport(0, 0, bitBuff->width, bitBuff->height);
	
	glEnable(GL_DEPTH_TEST);
	
	glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	
	glMatrixMode(GL_PROJECTION);
	ASSERT(bitBuff->width && bitBuff->height);
	float proj_mat[] = 
	{
		 2,  0,  0,  0,
		 0,  2,  0,  0,
		 0,  0,  1.0f/1000,  0,
		-1, -1,  -1,  1
	};
	glLoadMatrixf(proj_mat);
	
	for (u32 offset = 0; offset < cmdBuff->used;)
	{
		CommandHeader *header = (CommandHeader*)(cmdBuff->base_memory + offset);
		offset += sizeof(*header);
		
		switch (header->command_type)
		{
			case COMMAND_TYPE_CLEAR:
			{
				Command_Clear *cmd = (Command_Clear*)(cmdBuff->base_memory + offset);
				glClearColor(cmd->color.r, cmd->color.g, cmd->color.b, cmd->color.a);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				offset += sizeof(*cmd);
			} break;
			
			case COMMAND_TYPE_SET_VERTICES:
			{
				Command_SetVertices *cmd = (Command_SetVertices*)(cmdBuff->base_memory + offset);
				// DO NOTHING (4 now)
				offset += sizeof(*cmd);
			} break;
			
			case COMMAND_TYPE_TRIANGLE:
			{
				Command_Triangle *cmd = (Command_Triangle*)(cmdBuff->base_memory + offset);
				
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, 2);
				if (cmd->tex->index == 0)
				{
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cmd->tex->width, cmd->tex->height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, cmd->tex->pixels);

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
					
					glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
					
					cmd->tex->index = 2;
				}
				glBegin(GL_TRIANGLES);
	
				glColor4f(cmd->color.r, cmd->color.g, cmd->color.b, cmd->color.a);
				
				for (int i = 0; i < 3; i++)
				{
					glTexCoord4f(cmd->tex_points[i].u/cmd->points[i].z, (1.0f-cmd->tex_points[i].v)/cmd->points[i].z, 0.0f, 1.0f/cmd->points[i].z);
					//glTexCoord2f(cmd->tex_points[i].u, cmd->tex_points[i].v);
					glVertex3f(cmd->points[i].x, cmd->points[i].y, cmd->points[i].z);
				}
				
				glEnd();
				//auto err = glGetError();
				//ASSERT(0);
				glDisable(GL_TEXTURE_2D);
				
				offset += sizeof(*cmd);
			} break;
			/*
			case COMMAND_TYPE_INIT_TEXTURE:
			{
				Command_InitTexture *cmd = (Command_InitTexture*)(cmdBuff->base_memory + offset);
				InitTexture_OpenGL();
				offset += sizeof(*cmd);
			} break;*/
			
			default: ASSERT(0); break;
		}
	}
}

void ProcessCommandBuffer_Software(PlatformBitBuffer *bitBuff, CommandBuffer *cmdBuff)
{
	float *depth_buffer = new float[bitBuff->width*bitBuff->height]{};
	for (int i = 0; i < bitBuff->width*bitBuff->height; i++)
	{
		depth_buffer[i] = 10000.0f;
	}
	//Mat4 proj_mat4 = MakeProjectionMat4(90.0f, 1, bitBuff->width, bitBuff->height, 0.01f, 1000.0f);
	Mat4 proj_mat4 = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
	
	uptr *verts = 0;
	i32 *strides = 0;
	
	for (u32 offset = 0; offset < cmdBuff->used;)
	{
		CommandHeader *header = (CommandHeader*)(cmdBuff->base_memory + offset);
		offset += sizeof(*header);
		
		switch (header->command_type)
		{
			case COMMAND_TYPE_CLEAR:
			{
				Command_Clear *cmd = (Command_Clear*)(cmdBuff->base_memory + offset);
				FillPlatformBitBuffer(bitBuff, MakeColor(255, cmd->color.r*255, cmd->color.g*255, cmd->color.b*255));
				offset += sizeof(*cmd);
			} break;
			
			case COMMAND_TYPE_SET_VERTICES:
			{
				Command_SetVertices *cmd = (Command_SetVertices*)(cmdBuff->base_memory + offset);
				verts = cmd->vertices;
				strides = cmd->strides;
				offset += sizeof(*cmd);
			} break;
			
			case COMMAND_TYPE_TRIANGLE:
			{
				Command_Triangle *cmd = (Command_Triangle*)(cmdBuff->base_memory + offset);
				
				//Project triangle
				/*Vec3 points[3];
				Vec3 tex_points[3];
				for (int i = 0; i < 3; i++)
				{
					Vec4 point = { cmd->points[i].x, cmd->points[i].y, cmd->points[i].z, 1.0f };
					point = point * proj_mat4;
					tex_points[i].x /= point.w;
					tex_points[i].y /= point.w;
					tex_points[i].z = 1.0f / point.w;
					points[i] = {point.x/point.w+0.5f, point.y/point.w+0.5f, point.z};
				}*/
				
				/*TextureTrianglef(bitBuff,
							    cmd->points[0].x,    cmd->points[0].y,
							cmd->tex_points[0].x,cmd->tex_points[0].y, cmd->tex_points[0].z,
							    cmd->points[1].x,    cmd->points[1].y,
							cmd->tex_points[1].x,cmd->tex_points[1].y, cmd->tex_points[1].z,
							    cmd->points[2].x,    cmd->points[2].y,
							cmd->tex_points[2].x,cmd->tex_points[2].y, cmd->tex_points[2].z,
							cmd->tex, depth_buffer, cmd->color);*/
				newTextureTriangle(bitBuff, cmd->points[0], cmd->points[1], cmd->points[2],
									cmd->tex_points[0], cmd->tex_points[1], cmd->tex_points[2],
									cmd->tex, depth_buffer, cmd->color);
				
				offset += sizeof(*cmd);
			} break;
			/*
			case COMMAND_TYPE_INIT_TEXTURE:
			{
				Command_InitTexture *cmd = (Command_InitTexture*)(cmdBuff->base_memory + offset);
				offset += sizeof(*cmd);
			} break;*/
			
			default: ASSERT(0); break;
		}
	}
	
	delete[] depth_buffer;
}
