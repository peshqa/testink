/*
platform_interface.h - defines functions or "services" that platform layer offers for platform independent layer to use
also defines some enums and macros
2024/01/01, peshqa
*/
#pragma once

//#include <string>
#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t  i8;
typedef int32_t b32;

typedef uintptr_t uptr;

// figure out if SIMD is supported
#ifdef _MSC_VER // ms compiler is being used
#define USING_SIMD_SSE 1
#else // assume it's clang
#ifdef __ARM_NEON
#define USING_SIMD_NEON 1
#endif //__ARM_NEON
#endif //_MSC_VER

#ifdef ASSERT_ENABLE
#define ASSERT(e) if(!(e)) { *(int*)0 = 0; }
#else
#define ASSERT(e)
#endif

#define ARRAY_LENGTH(a) (sizeof(a) / sizeof(a[0]))

#define KILOBYTES(b)  (1024*(b)            )
#define MEGABYTES(kb) (1024*(KILOBYTES(kb)))
#define GIGABYTES(mb) (1024*(MEGABYTES(mb)))
#define TERABYTES(gb) (1024*(GIGABYTES(gb)))

#define INPUT_KEY_COUNT 300

enum CommandType : u32
{
	COMMAND_TYPE_CLEAR,
	COMMAND_TYPE_TRIANGLE
};

typedef struct
{
	u32 cmd_count;
	u8 *base_memory;
	u32 max_size;
	u32 used;
} CommandBuffer;

enum InputCode : unsigned int
{
	INPUT_LEFT = 0x25,
	INPUT_UP = 0x26,
	INPUT_RIGHT = 0x27,
	INPUT_DOWN = 0x28,
	INPUT_F1 = 0x70,
	INPUT_F2,
	INPUT_F3,
	INPUT_F4,
	INPUT_F5,
	INPUT_F6,
	INPUT_F7,
	INPUT_F8,
	INPUT_F9,
	INPUT_F10,
	INPUT_F11,
	INPUT_F12,
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
	int stride;
	void *bits;
	void *info;
	b32 is_top_to_bottom;
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
	
	unsigned char input_state[INPUT_KEY_COUNT];
	
	int is_running;
	int max_fps;
	
	int screen_mode;
	
	unsigned char is_accelerometer_active;
	unsigned char is_gyroscope_active;
	float rot_vec_values[4];
	
	CommandBuffer cmdBuff;
	int texture_count;
	
	void *extra;
};

// Functions to be implemented by the OS abstraction layer

int PlatformGoBorderlessFullscreen(SharedState *s);
u32 PlatformReadWholeFile(SharedState *s, char *filename, void *&p);
int PlatformFreeFileMemory(SharedState *s, void *p);

// TODO: to be removed/reimplemented
/*int OpenAssetFileA(SharedState *s, std::string &filename);
int ReadAssetLineA(SharedState *s, std::string &line);
int ReadAssetBytesA(SharedState *s, char *buf, unsigned int bytes);
int ReadAssetUntilSpaceA(SharedState *s, std::string &line);
int CloseAssetFile(SharedState *s);*/

int MakeColor(int a, int r, int g, int b);
//int PlatformDrawPixel(PlatformBitBuffer *bitBuff, int x, int y, int color);
int ResizePlatformBitBuffer(PlatformBitBuffer *p, int screenWidth, int screenHeight);
//int GrayscalePlatformBitBuffer(PlatformBitBuffer *bitBuff);
//int RedPlatformBitBuffer(PlatformBitBuffer *bitBuff);
