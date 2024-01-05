/*
platform_interface.h - defines functions or "services" that platform layer offers for platform independent layer to use
also defines some enums and macros
2024/01/01, peshqa
*/
#pragma once

#include <chrono>
#include <string>

#ifdef ASSERT_ENABLE
#define ASSERT(e) if(!(e)) { *(int*)0 = 0; }
#else
#define ASSERT(e)
#endif

enum InputCode : unsigned int
{
	INPUT_LEFT = 0x25,
	INPUT_UP = 0x26,
	INPUT_RIGHT = 0x27,
	INPUT_DOWN = 0x28,
	INPUT_LSHIFT = 0xA0,
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
	void *project_state;
	std::chrono::steady_clock::time_point curr_time;
	std::chrono::steady_clock::time_point prev_time;
	
	std::string asset_path;
	
	int is_lmb_down;
	int mouse_x;
	int mouse_y;
	
	unsigned char input_state[300];
	
	int is_running;
	int max_fps;
	
	int screen_mode;
	
	unsigned char is_accelerometer_active;
	unsigned char is_gyroscope_active;
	float rot_vec_values[4];
	
	void *extra;
};

// Functions to be implemented by the OS abstraction layer

int PlatformGoBorderlessFullscreen(SharedState *s);

int OpenAssetFileA(SharedState *s, std::string &filename);
int ReadAssetLineA(SharedState *s, std::string &line);
int ReadAssetBytesA(SharedState *s, char *buf, unsigned int bytes);
int ReadAssetUntilSpaceA(SharedState *s, std::string &line);
int CloseAssetFile(SharedState *s);

int MakeColor(int a, int r, int g, int b);
int PlatformDrawPixel(PlatformBitBuffer *bitBuff, int x, int y, int color);
int ResizePlatformBitBuffer(PlatformBitBuffer *p, int screenWidth, int screenHeight);
int GrayscalePlatformBitBuffer(PlatformBitBuffer *bitBuff);
int RedPlatformBitBuffer(PlatformBitBuffer *bitBuff);
