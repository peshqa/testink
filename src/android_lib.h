#pragma once
#include "snake_game.h"
#include <stdlib.h>
#include <chrono>
#include <string>
#include <vector>

typedef struct
{
	//W32BitBuffer* bitBuff;
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
} SharedState;

float CalculateDeltaTime(SharedState* state)
{
	state->prev_time = state->curr_time;
	state->curr_time = std::chrono::steady_clock::now();
	std::chrono::duration<float> dur = state->curr_time - state->prev_time;
	float elapsedTime = dur.count();
	return elapsedTime;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_peshqa_testink_GameEngine_initData(
		JNIEnv *env,
		jobject thiz)
{
	SharedState *state = new SharedState{};
	SnakeGameState* snake_game = new SnakeGameState{};
	InitSnakeGame(snake_game, 10, 10);
	state->project_state = snake_game;
	
	CalculateDeltaTime(state);
	return (jlong)state;
}

int Win32FillRect(void *pixels, int w, int h, int left, int top, int right, int bottom, int color)
{
	for (int y = top; y < bottom; y++)
	{
		for (int x = left; x < right; x++)
		{
			int pos = y*w+x;
			if (pos >=0 && pos < h*w)
				((int*)pixels)[pos] = color;
		}
	}
	return 0;
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

extern "C" JNIEXPORT void JNICALL
Java_com_peshqa_testink_GameEngine_itHappen(
		JNIEnv *env,
		jobject thiz,
		jlong data,
		jchar dir)
{
	SharedState* state = (SharedState*)data;
	state->dir = (char)dir;
}

extern "C" JNIEXPORT void JNICALL
Java_com_peshqa_testink_GameEngine_updateAndRenderGame(
		JNIEnv *env,
		jobject thiz,
		jlong data,
		jobject bitmap)
{
	AndroidBitmapInfo info;
	void *pixels;
	
	// Get info about the bitmap
	if (AndroidBitmap_getInfo(env, bitmap, &info) < 0)
	{
		return;
	}
	
	if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
	{
		return;
	}
	
	// Lock the bitmap to access its pixels
	if (AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0)
	{
		return;
	}
	
	SharedState* state = (SharedState*)data;
	SnakeGameState *game_state = (SnakeGameState*)(state->project_state);
	
	float delta_time = CalculateDeltaTime(state);
	
	if (state->dir == 'l')
	{
		game_state->snake_direction.x = -1;
		game_state->snake_direction.y = 0;
	} else if (state->dir == 'r')
	{
		game_state->snake_direction.x = 1;
		game_state->snake_direction.y = 0;
	} else if (state->dir == 'u')
	{
		game_state->snake_direction.x = 0;
		game_state->snake_direction.y = -1;
	} else if (state->dir == 'd')
	{
		game_state->snake_direction.x = 0;
		game_state->snake_direction.y = 1;
	}
	
	game_state->time_since_last_update += delta_time;
	if (game_state->time_since_last_update >= game_state->update_interval)
	{
		game_state->time_since_last_update -= game_state->update_interval;
		UpdateSnakeGameState(game_state);
	}
	
	//FillW32BitBuffer(state->bitBuff, 0x00000000);
	for (int i = 0; i < info.height; i++)
	{
		for (int j = 0; j < info.width; j++)
		{
			// Format: -RRGGBBAA- AABBGGRR
			((int32_t*)(pixels))[i*info.width+j] = 0xFF000000;
		}
	}
	
	int start_x{};
	int end_x = info.width;
	int start_y{};
	int end_y = info.height;
	if (info.width >= info.height)
	{
		start_x = 0.5f*(info.width - info.height);
		end_x = start_x + info.height;
	} else {
		start_y = 0.5f*(info.height - info.width);
		end_y = start_y + info.width;
	}
	
	// render background
	Win32FillRect(pixels, info.width, info.height,
		ConvertRelXToXse(0.0f, start_x, end_x),
		ConvertRelYToYse(0.0f, start_y, end_y),
		ConvertRelXToXse(1.0f, start_x, end_x),
		ConvertRelYToYse(1.0f, start_y, end_y),
		0xFF242424);
	for (int i = 0; i < game_state->field_height; i++)
	{
		for (int j = 0; j < game_state->field_width; j++)
		{
			if (((i+j)&1)==1)
			{
				continue;
			}
			Win32FillRect(pixels, info.width, info.height,
				ConvertRelXToXse((float)j/(float)game_state->field_width, start_x, end_x),
				ConvertRelYToYse((float)i/(float)game_state->field_height, start_y, end_y),
				ConvertRelXToXse((float)(j+1)/(float)game_state->field_width, start_x, end_x),
				ConvertRelYToYse((float)(i+1)/(float)game_state->field_height, start_y, end_y),
				0xFF202020);
		}
	}
	
	// render snake
	DoubleLinkedNode* node = game_state->snake_segments.first;
	int odd = 0;
	while (node)
	{
		Point2i* p = (Point2i*)(node->data);
		Win32FillRect(pixels, info.width, info.height,
			ConvertRelXToXse((float)p->x/(float)game_state->field_width, 		start_x, end_x),
			ConvertRelYToYse((float)p->y/(float)game_state->field_height, 		start_y, end_y),
			ConvertRelXToXse((float)(p->x+1)/(float)game_state->field_width, 	start_x, end_x),
			ConvertRelYToYse((float)(p->y+1)/(float)game_state->field_height, 	start_y, end_y),
			(odd) ? 0xFF10EE10 : 0xFF108810);
		odd = 1 - odd;
		node = node->next;
	}
	// render food
	node = game_state->fruits.first;
	while (node)
	{
		Point2i* p = (Point2i*)(node->data);
		Win32FillRect(pixels, info.width, info.height,
			ConvertRelXToXse((float)p->x/(float)game_state->field_width, 		start_x, end_x),
			ConvertRelYToYse((float)p->y/(float)game_state->field_height, 		start_y, end_y),
			ConvertRelXToXse((float)(p->x+1)/(float)game_state->field_width, 	start_x, end_x),
			ConvertRelYToYse((float)(p->y+1)/(float)game_state->field_height, 	start_y, end_y),
			0xFF1111FF);
		node = node->next;
	}
	
	AndroidBitmap_unlockPixels(env, bitmap);
	return;
}