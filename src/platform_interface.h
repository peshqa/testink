/*
platform_interface.h - defines functions or "services" that platform layer offers for platform independent layer to use
also defines some enums and macros
2024/01/01, peshqa
*/
#pragma once

#include <string>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t  i8;

#ifdef ASSERT_ENABLE
#define ASSERT(e) if(!(e)) { *(int*)0 = 0; }
#else
#define ASSERT(e)
#endif

#define ARRAY_LENGTH(a) sizeof(a) / sizeof(a[0])

#define KILOBYTES(b)  (1024*(b)            )
#define MEGABYTES(kb) (1024*(KILOBYTES(kb)))
#define GIGABYTES(mb) (1024*(MEGABYTES(mb)))
#define TERABYTES(gb) (1024*(GIGABYTES(gb)))

#define PI32 3.14159265359f

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

struct PlatformSoundBuffer
{
	void *buffer;
	int samples_per_sec;
	int sample_size;
	int num_channels;
	
	int samples_to_fill;
};
struct SharedState
{
	PlatformBitBuffer* bitBuff;
	PlatformSoundBuffer soundBuff;
	void *callback_update_func;
	int client_width;
	int client_height;
	int scale;
	char dir;
	void *project_memory;
	size_t project_memory_size;
	
	float delta_time;
	
	char *asset_path; // TODO: is this a good idea?
	
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
u32 PlatformReadWholeFile(SharedState *s, char *filename, void *&p);
int PlatformFreeFileMemory(void *p);

// TODO: to be removed/reimplemented
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
