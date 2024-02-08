/*
project.h - header file contains information about different projects
Every project has a name and function pointers to initiate and update its state.
Projects are provided with pointers to shared app state and can call functions to draw on screen.
2023/07/30, peshqa
*/
#pragma once

#include "platform_simple_renderer.h"
#include "snake_game.h"
#include "project_cube3d.h"
#include "project_image_render.h"

typedef int (*InitProjectFunction)(SharedState*);
typedef int (*UpdateProjectFunction)(SharedState*);

typedef struct
{
	wchar_t *name;
	//int (*InitFunc)(int, int);
	void *InitFunc;
	void *UpdateFunc;
} Project;

int InitProjectSnakeGame(SharedState* state)
{
	SnakeGameState* snake_game = (SnakeGameState*)(state->project_memory);
	InitSnakeGame(snake_game, 10, 10);
	
	return 0;
}

int UpdateProjectSnakeGame(SharedState* state)
{
	SnakeGameState *game_state = (SnakeGameState*)(state->project_memory);
	
	float delta_time = state->delta_time;
	
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
	
	FillPlatformBitBuffer(state->bitBuff, MakeColor(255,0,0,0));
	
	int start_x{};
	int end_x = state->client_width;
	int start_y{};
	int end_y = state->client_height;
	if (state->client_width >= state->client_height)
	{
		start_x = 0.5f*(state->client_width - state->client_height);
		end_x = start_x + state->client_height;
	} else {
		start_y = 0.5f*(state->client_height - state->client_width);
		end_y = start_y + state->client_width;
	}
	
	// render background
	PlatformFillRect(state->bitBuff,
		ConvertRelXToXse(0.0f, start_x, end_x),
		ConvertRelYToYse(0.0f, start_y, end_y),
		ConvertRelXToXse(1.0f, start_x, end_x),
		ConvertRelYToYse(1.0f, start_y, end_y),
		MakeColor(255,0x25,0x25,0x25));
	for (int i = 0; i < game_state->field_height; i++)
	{
		for (int j = 0; j < game_state->field_width; j++)
		{
			if (((i+j)&1)==1)
			{
				continue;
			}
			PlatformFillRect(state->bitBuff,
				ConvertRelXToXse((float)j/(float)game_state->field_width, start_x, end_x),
				ConvertRelYToYse((float)i/(float)game_state->field_height, start_y, end_y),
				ConvertRelXToXse((float)(j+1)/(float)game_state->field_width, start_x, end_x),
				ConvertRelYToYse((float)(i+1)/(float)game_state->field_height, start_y, end_y),
				MakeColor(255,0x20,0x20,0x20));
		}
	}
	
	// render snake
	DoubleLinkedNode* node = game_state->snake_segments.first;
	int odd = 0;
	while (node)
	{
		Point2i* p = (Point2i*)(node->data);
		PlatformFillRect(state->bitBuff,
			ConvertRelXToXse((float)p->x/(float)game_state->field_width, 		start_x, end_x),
			ConvertRelYToYse((float)p->y/(float)game_state->field_height, 		start_y, end_y),
			ConvertRelXToXse((float)(p->x+1)/(float)game_state->field_width, 	start_x, end_x),
			ConvertRelYToYse((float)(p->y+1)/(float)game_state->field_height, 	start_y, end_y),
			(odd) ? MakeColor(255,0x10,0xEE,0x10) : MakeColor(255,0x10,0x88,0x10));
		odd = 1 - odd;
		node = node->next;
	}
	// render food
	node = game_state->fruits.first;
	while (node)
	{
		Point2i* p = (Point2i*)(node->data);
		PlatformFillRect(state->bitBuff,
			ConvertRelXToXse((float)p->x/(float)game_state->field_width, 		start_x, end_x),
			ConvertRelYToYse((float)p->y/(float)game_state->field_height, 		start_y, end_y),
			ConvertRelXToXse((float)(p->x+1)/(float)game_state->field_width, 	start_x, end_x),
			ConvertRelYToYse((float)(p->y+1)/(float)game_state->field_height, 	start_y, end_y),
			MakeColor(255,0xFF,0x11,0x11));
		node = node->next;
	}
	
	//GrayscaleW32BitBuffer(state->bitBuff);
	//RedW32BitBuffer(state->bitBuff);
	
	return 0;
}

int InitProjectArray(Project* &array)
{
	// TODO: automate this count OR set the count to a high number instead?
	int count = 3;
	array = new Project[count];
	// First element of the array points to default project
	array[0].InitFunc = (void*)InitProjectImageRender;
	array[0].UpdateFunc = (void*)UpdateProjectImageRender;
	
	// TODO array[0].name = ...
	array[1].InitFunc = (void*)InitProjectSnakeGame;
	array[1].UpdateFunc = (void*)UpdateProjectSnakeGame;
	
	array[2].InitFunc = (void*)InitProject3DCube;
	array[2].UpdateFunc = (void*)UpdateProject3DCube;
	
	return 0;
}