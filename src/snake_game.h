/*
snake_game.h - good old snake game made for coding practice
2023/07/29, peshqa
*/
#pragma once
#include "double_linked_list.h"

typedef struct
{
	int x;
	int y;
} Point2i;

typedef struct
{
	unsigned int field_width;
	unsigned int field_height;
	DoubleLinkedList snake_segments;
	DoubleLinkedList fruits;
	Point2i snake_direction;
} SnakeGameState;

int InitSnakeGame(SnakeGameState *state, int field_width, int field_height)
{
	InitDoubleLinkedList(&state->snake_segments);
	InitDoubleLinkedList(&state->fruits);
	state->field_width = field_width;
	state->field_height = field_height;
	
	Point2i* p1 = new Point2i{field_width/2, field_height/2};
	Point2i* p2 = new Point2i{p1->x+1, p1->y};
	AddNodeAtBack(&state->snake_segments, p1);
	AddNodeAtBack(&state->snake_segments, p2);
	
	p1 = new Point2i{1, 1};
	AddNodeAtBack(&state->fruits, p1);
	
	state->snake_direction.x = -1;
	state->snake_direction.y = 0;
	
	return 0;
}

int InitSnakeGame(SnakeGameState *state)
{
	return InitSnakeGame(state, 10, 10);
}

int UpdateSnakeGameState(SnakeGameState *state)
{
	DoubleLinkedNode* h = state->snake_segments.first;
	Point2i* head = (Point2i*)(state->snake_segments.first->data);
	Point2i* p1 = new Point2i{head->x + state->snake_direction.x, head->y + state->snake_direction.y};
	
	if (p1->x < 0 || p1->x >= state->field_width || p1->y < 0 || p1->y >= state->field_height)
	{
		delete p1;
		return 1; // out of bounds
	}
	
	BOOL occupied_by_snake = 0;
	DoubleLinkedNode* node = state->snake_segments.first;
	while (node->next)
	{
		Point2i* p = (Point2i*)(node->data);
		if (p->x == p1->x && p->y == p1->y)
		{
			occupied_by_snake = 1;
			break;
		}
		node = node->next;
	}
	
	if (occupied_by_snake == 1)
	{
		delete p1;
		return 2; // snake has bitten itself :(
	}
	
	BOOL occupied_by_fruit = 0;
	node = state->fruits.first;
	while (node)
	{
		Point2i* p = (Point2i*)(node->data);
		if (p->x == p1->x && p->y == p1->y)
		{
			occupied_by_fruit = 1;
			break;
		}
		node = node->next;
	}
	
	
	AddNodeAtFront(&state->snake_segments, p1);
	if (occupied_by_fruit == 0)
	{
		Point2i* p = (Point2i*)RemoveNodeAtBack(&state->snake_segments);
		delete p;
	} else {
		Point2i* p = (Point2i*)(state->fruits.first->data);
		p->x = rand() % state->field_width;
		p->y = rand() % state->field_height;
	}
	
	
	return 0;
}

