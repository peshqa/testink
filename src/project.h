/*
project.h - header file contains information about different projects
Every project has a name and function pointers to initiate and update its state.
Projects are provided with pointers to shared app state and can call functions to draw on screen.
2023/07/30, peshqa
*/
#pragma once

#include "simple_win32_renderer.h"
#include "snake_game.h"

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
	SnakeGameState* snake_game = new SnakeGameState{};
	InitSnakeGame(snake_game, 10, 10);
	state->project_state = snake_game;
	
	CalculateDeltaTime(state);
	
	return 0;
}

int UpdateProjectSnakeGame(SharedState* state)
{
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
	
	FillW32BitBuffer(state->bitBuff, 0x00222222);
	DoubleLinkedNode* node = game_state->snake_segments.first;
	int odd = 0;
	while (node)
	{
		Point2i* p = (Point2i*)(node->data);
		Win32FillRect(state->bitBuff,
			ConvertRelXToX((float)p->x/(float)game_state->field_width, state->bitBuff),
			ConvertRelYToY((float)p->y/(float)game_state->field_height, state->bitBuff),
			ConvertRelXToX((float)(p->x+1)/(float)game_state->field_width, state->bitBuff)-1,
			ConvertRelYToY((float)(p->y+1)/(float)game_state->field_height, state->bitBuff)-1,
			(odd) ? COLOR_GREEN : 0x00008800);
		odd = 1 - odd;
		node = node->next;
	}
	node = game_state->fruits.first;
	while (node)
	{
		Point2i* p = (Point2i*)(node->data);
		Win32FillRect(state->bitBuff,
			ConvertRelXToX((float)p->x/(float)game_state->field_width, state->bitBuff),
			ConvertRelYToY((float)p->y/(float)game_state->field_height, state->bitBuff),
			ConvertRelXToX((float)(p->x+1)/(float)game_state->field_width, state->bitBuff)-1,
			ConvertRelYToY((float)(p->y+1)/(float)game_state->field_height, state->bitBuff)-1,
			COLOR_RED);
		node = node->next;
	}
	
	return 0;
}

int InitProjectArray(Project* &array)
{
	int count = 1;
	array = new Project[count];
	// TODO array[0].name = ...
	array[0].InitFunc = InitProjectSnakeGame;
	array[0].UpdateFunc = UpdateProjectSnakeGame;
	
	return 0;
}