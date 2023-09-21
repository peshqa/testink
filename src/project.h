/*
project.h - header file contains information about different projects
Every project has a name and function pointers to initiate and update its state.
Projects are provided with pointers to shared app state and can call functions to draw on screen.
2023/07/30, peshqa
*/
#pragma once

#include <math.h>
#include "platform_simple_renderer.h"
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

typedef struct
{
	float x_offset;
	float y_offset;
	float z_offset;
} ProjectState3DCube;

int InitProject3DCube(SharedState* state)
{
	ProjectState3DCube *p_state = new ProjectState3DCube{};
	p_state->x_offset = 0.0f;
	p_state->y_offset = 0.0f;
	p_state->z_offset = 2.0f;
	state->project_state = p_state;
	CalculateDeltaTime(state);
	return 0;
}

typedef struct
{
	float x;
	float y;
	float z;
} Vec3f;

typedef struct
{
	Vec3f p1;
	Vec3f p2;
	Vec3f p3;
} Tri3f;

typedef struct
{
	std::vector<Tri3f> mesh;
} Mesh3f;

int UpdateProject3DCube(SharedState* state)
{
	ProjectState3DCube *game_state = (ProjectState3DCube*)(state->project_state);
	
	float delta_time = CalculateDeltaTime(state);
	
	int fov_deg = 90;
	int z_near = 1;
	int z_far = 10;
	
	FillPlatformBitBuffer(state->bitBuff, MakeColor(255,25,12,6));
	
	game_state->x_offset += 0.01f*delta_time;
	game_state->y_offset+= 0.005f *delta_time;
	game_state->z_offset-= 0.01f *delta_time;
	float ox = game_state->x_offset;
	float oy = game_state->y_offset;
	float oz = game_state->z_offset;
	std::vector<Tri3f> mesh;
	// counterclockwise order (maybe)
	Tri3f t1 = {1.0f+ox, 0.0f+oy, 0.0f+oz,	0.0f+ox, 1.0f+oy, 0.0f+oz,	1.0f+ox, 1.0f+oy, 0.0f+oz};
	Tri3f t2 = {1.0f+ox, 0.0f+oy, 0.0f+oz,	0.0f+ox, 0.0f+oy, 0.0f+oz,	0.0f+ox, 1.0f+oy, 0.0f+oz};
	Tri3f t3 = {1.0f+ox, 0.0f+oy, 1.0f+oz,	0.0f+ox, 1.0f+oy, 1.0f+oz,	1.0f+ox, 1.0f+oy, 1.0f+oz};
	Tri3f t4 = {1.0f+ox, 0.0f+oy, 1.0f+oz,	0.0f+ox, 0.0f+oy, 1.0f+oz,	0.0f+ox, 1.0f+oy, 1.0f+oz};
	Tri3f t5 = {0.0f+ox, 1.0f+oy, 0.0f+oz,	0.0f+ox, 0.0f+oy, 1.0f+oz,	0.0f+ox, 1.0f+oy, 1.0f+oz};
	Tri3f t6 = {0.0f+ox, 1.0f+oy, 0.0f+oz,	0.0f+ox, 0.0f+oy, 0.0f+oz,	0.0f+ox, 0.0f+oy, 1.0f+oz};
	Tri3f t7 = {1.0f+ox, 1.0f+oy, 0.0f+oz,	1.0f+ox, 0.0f+oy, 1.0f+oz,	1.0f+ox, 1.0f+oy, 1.0f+oz};
	Tri3f t8 = {1.0f+ox, 1.0f+oy, 0.0f+oz,	1.0f+ox, 0.0f+oy, 0.0f+oz,	1.0f+ox, 0.0f+oy, 1.0f+oz};
	Tri3f t9 = {1.0f+ox, 1.0f+oy, 0.0f+oz,	0.0f+ox, 1.0f+oy, 1.0f+oz,	1.0f+ox, 1.0f+oy, 1.0f+oz};
	Tri3f t10= {1.0f+ox, 1.0f+oy, 0.0f+oz,	0.0f+ox, 1.0f+oy, 0.0f+oz,	0.0f+ox, 1.0f+oy, 1.0f+oz};
	Tri3f t11= {1.0f+ox, 0.0f+oy, 0.0f+oz,	0.0f+ox, 0.0f+oy, 1.0f+oz,	1.0f+ox, 0.0f+oy, 1.0f+oz};
	Tri3f t12= {1.0f+ox, 0.0f+oy, 0.0f+oz,	0.0f+ox, 0.0f+oy, 0.0f+oz,	0.0f+ox, 0.0f+oy, 1.0f+oz};
	mesh.push_back(t1); mesh.push_back(t2);
	mesh.push_back(t3); mesh.push_back(t4);
	mesh.push_back(t5); mesh.push_back(t6);
	mesh.push_back(t7); mesh.push_back(t8);
	mesh.push_back(t9); mesh.push_back(t10);
	mesh.push_back(t11); mesh.push_back(t12);
	
	double pi = 3.1415926535;
	int clr = MakeColor(255,255,255,255);
	for (Tri3f t: mesh)
	{
		PlatformDrawLine(state->bitBuff,
			ConvertRelXToXse((t.p1.x*state->client_height/state->client_width)/t.p1.z, 0, state->client_width),
			ConvertRelYToYse(t.p1.y/t.p1.z, 0, state->client_height),
			ConvertRelXToXse((t.p2.x*state->client_height/state->client_width)/t.p2.z, 0, state->client_width),
			ConvertRelYToYse(t.p2.y/t.p2.z, 0, state->client_height),
			clr);
		PlatformDrawLine(state->bitBuff,
			ConvertRelXToXse((t.p3.x*state->client_height/state->client_width)/t.p3.z, 0, state->client_width),
			ConvertRelYToYse(t.p3.y/t.p3.z, 0, state->client_height),
			ConvertRelXToXse((t.p2.x*state->client_height/state->client_width)/t.p2.z, 0, state->client_width),
			ConvertRelYToYse(t.p2.y/t.p2.z, 0, state->client_height),
			clr);
		PlatformDrawLine(state->bitBuff,
			ConvertRelXToXse((t.p1.x*state->client_height/state->client_width)/t.p1.z, 0, state->client_width), 
			ConvertRelYToYse(t.p1.y/t.p1.z, 0, state->client_height),
			ConvertRelXToXse((t.p3.x*state->client_height/state->client_width)/t.p3.z, 0, state->client_width), 
			ConvertRelYToYse(t.p3.y/t.p3.z, 0, state->client_height), 
			clr);
	}
	
	PlatformDrawLine(state->bitBuff,
			ConvertRelXToXse(((0.0f+ox)*state->client_height/state->client_width)/oz, 0, state->client_width), 
			ConvertRelYToYse((0.0f+oy)/oz, 0, state->client_height),
			ConvertRelXToXse(((1.0f+ox)*state->client_height/state->client_width)/oz, 0, state->client_width), 
			ConvertRelYToYse((0.0f+oy)/oz, 0, state->client_height), 
			MakeColor(255,255,0,0));
	PlatformDrawLine(state->bitBuff,
			ConvertRelXToXse(((0.0f+ox)*state->client_height/state->client_width)/oz, 0, state->client_width), 
			ConvertRelYToYse((0.0f+oy)/oz, 0, state->client_height),
			ConvertRelXToXse(((0.0f+ox)*state->client_height/state->client_width)/oz, 0, state->client_width), 
			ConvertRelYToYse((1.0f+oy)/oz, 0, state->client_height), 
			MakeColor(255,0,255,0));
	PlatformDrawLine(state->bitBuff,
			ConvertRelXToXse(((0.0f+ox)*state->client_height/state->client_width)/oz, 0, state->client_width), 
			ConvertRelYToYse((0.0f+oy)/oz, 0, state->client_height),
			ConvertRelXToXse(((0.0f+ox)*state->client_height/state->client_width)/oz, 0, state->client_width), 
			ConvertRelYToYse((0.0f+oy)/(oz+1.0f), 0, state->client_height), 
			MakeColor(255,0,0,255));
	
	return 0;
}

int InitProjectArray(Project* &array)
{
	int count = 2;
	array = new Project[count];
	// TODO array[0].name = ...
	array[1].InitFunc = (void*)InitProjectSnakeGame;
	array[1].UpdateFunc = (void*)UpdateProjectSnakeGame;
	
	array[0].InitFunc = (void*)InitProject3DCube;
	array[0].UpdateFunc = (void*)UpdateProject3DCube;
	
	return 0;
}