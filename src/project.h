/*
project.h - header file contains information about different projects
Every project has a name and function pointers to initiate and update its state.
Projects are provided with pointers to shared app state and can call functions to draw on screen.
2023/07/30, peshqa
*/
#pragma once


#include "platform_simple_renderer.h"
#include "snake_game.h"

#include <assert.h>

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
	
	int last_mouse_x;
	int last_mouse_y;
	int was_lmb_down;
	
	float pitch;
	float yaw;
	float roll;
	
	float cube_yaw;
	float cube_pitch;
} ProjectState3DCube;

int InitProject3DCube(SharedState* state)
{
	//PlatformGoBorderlessFullscreen(state);
	ProjectState3DCube *p_state = new ProjectState3DCube{};
	p_state->x_offset = 0;//0.5f;
	p_state->y_offset = 0;//0.5f;
	p_state->z_offset = 0;//2.0f;
	
	p_state->last_mouse_x = 0;
	p_state->last_mouse_y = 0;
	p_state->was_lmb_down = 0;
	
	p_state->pitch = 0.0f;
	p_state->yaw   = 0.0f;
	p_state->roll  = 0.0f;
	
	p_state->cube_yaw   = 0.0f;
	p_state->cube_pitch = 0.0f;
	
	state->project_state = p_state;
	CalculateDeltaTime(state);
	return 0;
}

int UpdateProject3DCube(SharedState* state)
{
	ProjectState3DCube *game_state = (ProjectState3DCube*)(state->project_state);
	float delta_time = CalculateDeltaTime(state);
	static float cam_rot = 0.0f;
	
	// Mouse input -> camera rotation control
	
	if (state->is_lmb_down == 1)
	{
		if (game_state->was_lmb_down == 0)
		{
			game_state->was_lmb_down = 1;
			game_state->last_mouse_x = state->mouse_x;
			game_state->last_mouse_y = state->mouse_y;
		} else {
			//game_state->x_offset += 0.02f*(-state->mouse_x+game_state->last_mouse_x);
			//game_state->y_offset += 0.02f*(-state->mouse_y+game_state->last_mouse_y);
			game_state->yaw   += 0.005f*(-state->mouse_x+game_state->last_mouse_x);
			game_state->pitch += 0.005f*(-state->mouse_y+game_state->last_mouse_y);
			game_state->last_mouse_x = state->mouse_x;
			game_state->last_mouse_y = state->mouse_y;
		}
		//game_state->x_offset += 1.0f*delta_time;
	} else {
		game_state->was_lmb_down = 0;
	}
	
	if (state->input_state[INPUT_LEFT] == 1)
	{
		game_state->cube_yaw -= delta_time*1;
	}
	if (state->input_state[INPUT_RIGHT] == 1)
	{
		game_state->cube_yaw += delta_time*1;
	}
	if (state->input_state[INPUT_DOWN] == 1)
	{
		game_state->cube_pitch -= delta_time*1;
	}
	if (state->input_state[INPUT_UP] == 1)
	{
		game_state->cube_pitch += delta_time*1;
	}

	float proj_mat4x4[16];
	InitProjectionMat4x4(proj_mat4x4, 90.0f, 1, state->bitBuff->width, state->bitBuff->height, 0.01f, 100.0f);
	
	float scale_mat4x4[16];
	//InitScaleMat4x4(scale_mat4x4, 0.3f, 0.3f, 0.3f);
	InitScaleMat4x4(scale_mat4x4, 1, 1, 1);
	
	float combined_mat4x4[16]{};
	float combined2_mat4x4[16]{};
	
	Vec3f target = {0.0f, 0.0f, 1.0f};
	Vec3f target_rot_y;
	Vec3f target_rot_yx;
	Vec3f target_final;
	float t_y_rot_mat4x4[16]{};
	float t_x_rot_mat4x4[16]{};
	InitXRotMat4x4(t_y_rot_mat4x4, -game_state->pitch);
	//InitXRotMat4x4(t_y_rot_mat4x4, 0);
	MultiplyVecMat4x4((float*)&target, t_y_rot_mat4x4, (float*)&target_rot_y);
	InitYRotMat4x4(t_x_rot_mat4x4, game_state->yaw);
	MultiplyVecMat4x4((float*)&target_rot_y, t_x_rot_mat4x4, (float*)&target_rot_yx);
	
	if (state->input_state['W'] == 1)
	{
		game_state->x_offset += target_rot_yx.x*delta_time*10;
		game_state->y_offset += target_rot_yx.y*delta_time*10;
		game_state->z_offset += target_rot_yx.z*delta_time*10;
	}
	if (state->input_state['A'] == 1)
	{
		game_state->z_offset += target_rot_yx.x*delta_time*10;
		//game_state->y_offset += target_rot_yx.y*delta_time*10;
		game_state->x_offset -= target_rot_yx.z*delta_time*10;
	}
	if (state->input_state['S'] == 1)
	{
		game_state->x_offset -= target_rot_yx.x*delta_time*10;
		game_state->y_offset -= target_rot_yx.y*delta_time*10;
		game_state->z_offset -= target_rot_yx.z*delta_time*10;
	}
	if (state->input_state['D'] == 1)
	{
		game_state->z_offset -= target_rot_yx.x*delta_time*10;
		//game_state->y_offset -= target_rot_yx.y*delta_time*10;
		game_state->x_offset += target_rot_yx.z*delta_time*10;
	}
	if (state->input_state[' '] == 1)
	{
		game_state->y_offset -= delta_time*2;
	}
	if (state->input_state[INPUT_LSHIFT] == 1)
	{
		game_state->y_offset += delta_time*2;
	}
	
	float ox = game_state->x_offset;
	float oy = game_state->y_offset;
	float oz = game_state->z_offset;
	Vec3f pos = {ox, oy, oz};
	Vec3fAdd(pos, target_rot_yx, target_final);
	
	Vec3f up = {0, 1, 0};
	float look_at_mat4x4[16];
	float point_at_mat4x4[16];
	InitPointAtMat4x4(point_at_mat4x4, pos, target_final, up);
	Mat4x4QuickInverse(point_at_mat4x4, look_at_mat4x4);
	
	float x_rot_mat4x4[16];
	float y_rot_mat4x4[16]{};
	float translate_mat4x4[16];
	InitXRotMat4x4(x_rot_mat4x4, game_state->cube_pitch);
	InitYRotMat4x4(y_rot_mat4x4, game_state->cube_yaw);
	InitTranslationMat4x4(translate_mat4x4, 0.0f, 0.0f, 3.0f);
	
	MultiplyMats4x4(scale_mat4x4, y_rot_mat4x4, combined_mat4x4);
	MultiplyMats4x4(combined_mat4x4, x_rot_mat4x4, combined2_mat4x4);
	MultiplyMats4x4(combined2_mat4x4, translate_mat4x4, combined_mat4x4);
	//MultiplyMats4x4(combined_mat4x4, look_at_mat4x4, combined2_mat4x4);
	
	FillPlatformBitBuffer(state->bitBuff, MakeColor(255,25,12,6));
	
	int clr = MakeColor(255,255,255,255);
	
	// Vertices of a cube
	float p0[] = {0.0f, 0.0f, 0.0f};
	float p1[] = {0.0f, 0.0f, 1.0f};
	float p2[] = {0.0f, 1.0f, 0.0f};
	float p3[] = {0.0f, 1.0f, 1.0f};
	float p4[] = {1.0f, 0.0f, 0.0f};
	float p5[] = {1.0f, 0.0f, 1.0f};
	float p6[] = {1.0f, 1.0f, 0.0f};
	float p7[] = {1.0f, 1.0f, 1.0f};
	
	std::vector<float*> vertices;
	vertices.push_back(p0);
	vertices.push_back(p1);
	vertices.push_back(p2);
	vertices.push_back(p3);
	vertices.push_back(p4);
	vertices.push_back(p5);
	vertices.push_back(p6);
	vertices.push_back(p7);
	
	// Triangles represented as point indices
	int tri1[] = {0, 1, 2};
	int tri2[] = {3, 1, 2};
	int tri3[] = {4, 5, 6};
	int tri4[] = {7, 5, 6};
	int tri5[] = {0, 6, 2};
	int tri6[] = {0, 6, 4};
	int tri7[] = {1, 7, 3};
	int tri8[] = {1, 7, 5};
	int tri9[] = {3, 2, 7};
	int tri10[] = {6, 2, 7};
	int tri11[] = {1, 0, 5};
	int tri12[] = {4, 0, 5};
	
	std::vector<int*> triangles;
	triangles.push_back(tri1); // left face
	triangles.push_back(tri2);
	triangles.push_back(tri3); // right face
	triangles.push_back(tri4);
	triangles.push_back(tri5); // front face
	triangles.push_back(tri6);
	triangles.push_back(tri7); // back face
	triangles.push_back(tri8);
	triangles.push_back(tri9); // top face
	triangles.push_back(tri10);
	triangles.push_back(tri11); // bottom face
	triangles.push_back(tri12);
	
	
	// Apply world transformations to vertices
	std::vector<float*> vertices_transformed;
	for (float *vx: vertices)
	{
		float v_in1[4] = {vx[0], vx[1], vx[2], 1};
		float v_out1[4];
		
		MultiplyVecMat4x4(v_in1, combined_mat4x4, v_out1);
		
		float *new_vx = new float[3]{v_out1[0], v_out1[1], v_out1[2]};
		vertices_transformed.push_back(new_vx);
	}
	
	// Apply view matrix
	std::vector<float*> vertices_viewed;
	for (float *vx: vertices_transformed)
	{
		float v_in1[4] = {vx[0], vx[1], vx[2], 1};
		float v_out1[4];
		
		MultiplyVecMat4x4(v_in1, look_at_mat4x4, v_out1);
		
		float *new_vx = new float[3]{v_out1[0], v_out1[1], v_out1[2]};
		vertices_viewed.push_back(new_vx);
	}
	
	// Project 3D vertices onto 2D screen
	std::vector<float*> vertices_projected;
	for (float *vx: vertices_viewed)
	{
		float v_in1[4] = {vx[0], vx[1], vx[2], 1};
		float v_out1[4];
		
		MultiplyVecMat4x4(v_in1, proj_mat4x4, v_out1);
		
		float *new_vx = new float[3]{v_out1[0]/v_out1[3], v_out1[1]/v_out1[3], v_out1[2]};
		//float *new_vx = new float[3]{(v_out1[0]+0.f)*0.001f*state->client_width, (v_out1[1]+0.f)*0.001f*state->client_height, v_out1[2]};
		vertices_projected.push_back(new_vx);
	}
	
	for (int *tri: triangles)
	{
		DrawTrianglef(state->bitBuff,
						vertices_projected[tri[0]][0]+0.5f, vertices_projected[tri[0]][1]+0.5f,
						vertices_projected[tri[1]][0]+0.5f, vertices_projected[tri[1]][1]+0.5f,
						vertices_projected[tri[2]][0]+0.5f, vertices_projected[tri[2]][1]+0.5f,
						clr);
		/*FillTrianglef(state->bitBuff,
						vertices_projected[tri[0]][0], vertices_projected[tri[0]][1],
						vertices_projected[tri[1]][0], vertices_projected[tri[1]][1],
						vertices_projected[tri[2]][0], vertices_projected[tri[2]][1],
						clr);*/
	}
	
	for (float *vx: vertices_transformed)
	{
		delete [] vx;
	}
	
	for (float *vx: vertices_viewed)
	{
		delete [] vx;
	}
	
	for (float *vx: vertices_projected)
	{
		delete [] vx;
	}
	
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