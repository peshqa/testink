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
	
	return 0;
}

int UpdateProjectSnakeGame(SharedState* state)
{
	SnakeGameState *game_state = (SnakeGameState*)(state->project_state);
	
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
	UpdateSnakeGameState(game_state);
	
	FillW32BitBuffer(state->bitBuff, 0x00111111);
	DoubleLinkedNode* node = game_state->snake_segments.first;
	while (node)
	{
		Point2i* p = (Point2i*)(node->data);
		Win32DrawPixel(state->bitBuff, p->x, p->y, COLOR_GREEN);
		node = node->next;
	}
	node = game_state->fruits.first;
	while (node)
	{
		Point2i* p = (Point2i*)(node->data);
		Win32DrawPixel(state->bitBuff, p->x, p->y, COLOR_RED);
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