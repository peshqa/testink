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
	p_state->x_offset = 0.5f;
	p_state->y_offset = 0.5f;
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
	Vec3f p[3];
} Tri3f;

void MultiplyVecMat4x4(float *v_in, float *mat, float *v_out)
{
	for (int col = 0; col < 4; col++)
	{
		v_out[col] = 0;
		for (int row = 0; row < 4; row++)
		{
			v_out[col] += mat[row*4+col]*v_in[row];
		}
	}
}
void MultiplyMats4x4(float *m1, float *m2, float *out)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			out[i*4+j] = 0;
			for (int k = 0; k < 4; k++)
			{
				out[i*4+j] += m1[i*4+k]*m2[k*4+j];
			}
		}
	}
}
void CrossProductVec3f(Vec3f &v1, Vec3f &v2, Vec3f &out)
{
	out.x = v1.y*v2.z - v1.z*v2.y;
	out.y = v1.z*v2.x - v1.x*v2.z;
	out.z = v1.x*v2.y - v1.y*v2.x;
}

int UpdateProject3DCube(SharedState* state)
{
	ProjectState3DCube *game_state = (ProjectState3DCube*)(state->project_state);
	float delta_time = CalculateDeltaTime(state);
	
	double pi = 3.1415926535;
	
	float fov_deg = 90.0f;
	float z_near = 0.1f;
	float z_far = 100.0f;
	float aspect_ratio = (float)state->bitBuff->height/state->bitBuff->width;
	float f = 1/tanf(fov_deg*pi/180/2);
	float q = z_far*(z_far-z_near);
	
	float ox = game_state->x_offset;
	float oy = game_state->y_offset;
	float oz = game_state->z_offset;
	
	float rot_speed = 1.0f;
	
	float proj_mat4x4[16] = {aspect_ratio*f,0.0f,0.0f,0.0f,	0.0f,f,0.0f,0.0f,	0.0f,0.0f,q,1.0f,	0.0f,0.0f,-q*z_near,0.0f};
	static float lol = 0;
	lol += delta_time*0.6f*2*rot_speed;
	float x_rot_mat4x4[16]{};
	x_rot_mat4x4[0] = 1;
	x_rot_mat4x4[5] = cosf(lol);
	x_rot_mat4x4[6] = -sinf(lol);
	x_rot_mat4x4[9] = sinf(lol);
	x_rot_mat4x4[10] = cosf(lol);
	x_rot_mat4x4[15] = 1;
	
	static float lol2{};
	lol2 += delta_time*0.2f*2*rot_speed;
	float y_rot_mat4x4[16]{};
	y_rot_mat4x4[0] = cosf(lol2);
	y_rot_mat4x4[2] = sinf(lol2);
	y_rot_mat4x4[5] = y_rot_mat4x4[15] = 1;
	y_rot_mat4x4[8] = -sinf(lol2);
	y_rot_mat4x4[10] = cosf(lol2);
	
	float translate_mat4x4[16]{};
	translate_mat4x4[0] = translate_mat4x4[5] = translate_mat4x4[10] = 1;
	translate_mat4x4[12] = ox;
	translate_mat4x4[13] = oy;
	translate_mat4x4[14] = oz;
	translate_mat4x4[15] = 1;
	
	float scale_mat4x4[16]{};
	scale_mat4x4[0] = 0.3f;
	scale_mat4x4[5] = 0.3f;
	scale_mat4x4[10] = 0.3f;
	scale_mat4x4[15] = 1;
	
	float combined_mat4x4[16]{};
	float combined2_mat4x4[16]{};
	/*combined_mat4x4[0] = 1;
	combined_mat4x4[5] = 1;
	combined_mat4x4[10] = 1;
	combined_mat4x4[15] = 1;*/
	
	
	MultiplyMats4x4(x_rot_mat4x4, y_rot_mat4x4, combined_mat4x4);
	MultiplyMats4x4(combined_mat4x4, scale_mat4x4, combined2_mat4x4);
	MultiplyMats4x4(combined2_mat4x4, translate_mat4x4, combined_mat4x4);
	MultiplyMats4x4(combined_mat4x4, proj_mat4x4, combined2_mat4x4);
	
	FillPlatformBitBuffer(state->bitBuff, MakeColor(255,25,12,6));
	
	/*game_state->x_offset += 0.01f*delta_time;
	game_state->y_offset+= 0.005f *delta_time;
	game_state->z_offset-= 0.01f *delta_time;*/
	
	std::vector<Tri3f> mesh;
	// clockwise order
	Tri3f t1 = {1.0f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f,	1.0f, 1.0f, 0.0f};//
	Tri3f t2 = {1.0f, 0.0f, 0.0f,	0.0f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f};//
	Tri3f t3 = {1.0f, 0.0f, 1.0f,	1.0f, 1.0f, 1.0f,	0.0f, 1.0f, 1.0f};//?
	Tri3f t4 = {1.0f, 0.0f, 1.0f,	0.0f, 1.0f, 1.0f,	0.0f, 0.0f, 1.0f};//?
	Tri3f t5 = {0.0f, 1.0f, 0.0f,	0.0f, 0.0f, 1.0f,	0.0f, 1.0f, 1.0f};//
	Tri3f t6 = {0.0f, 1.0f, 0.0f,	0.0f, 0.0f, 0.0f,	0.0f, 0.0f, 1.0f};//
	Tri3f t7 = {1.0f, 1.0f, 0.0f,	1.0f, 1.0f, 1.0f,	1.0f, 0.0f, 1.0f};//?
	Tri3f t8 = {1.0f, 1.0f, 0.0f,	1.0f, 0.0f, 1.0f,	1.0f, 0.0f, 0.0f};//?
	Tri3f t9 = {1.0f, 1.0f, 0.0f,	0.0f, 1.0f, 1.0f,	1.0f, 1.0f, 1.0f};//
	Tri3f t10= {1.0f, 1.0f, 0.0f,	0.0f, 1.0f, 0.0f,	0.0f, 1.0f, 1.0f};//
	Tri3f t11= {1.0f, 0.0f, 0.0f,	1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f};//?
	Tri3f t12= {1.0f, 0.0f, 0.0f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f, 0.0f};//?
	mesh.push_back(t1); mesh.push_back(t2);
	mesh.push_back(t3); mesh.push_back(t4);
	mesh.push_back(t5); mesh.push_back(t6);
	mesh.push_back(t7); mesh.push_back(t8);
	mesh.push_back(t9); mesh.push_back(t10);
	mesh.push_back(t11); mesh.push_back(t12);
	
	std::vector<Tri3f> translated_mesh;//?
	int clr = MakeColor(255,255,255,255);
	for (Tri3f t: mesh)
	{
		Tri3f tra_t;
		Tri3f projected_tri;
		for (int i = 0; i < 3; i++)
		{
			float v_in1[4] = {t.p[i].x, t.p[i].y, t.p[i].z, 1};
			float v_out1[4];

			MultiplyVecMat4x4(v_in1, combined_mat4x4, v_out1);
			
			tra_t.p[i].x = v_out1[0];
			tra_t.p[i].y = v_out1[1];
			tra_t.p[i].z = v_out1[2];
		}
		
		Vec3f norm;
		Vec3f v1 = {tra_t.p[1].x-tra_t.p[0].x, tra_t.p[1].y-tra_t.p[0].y, tra_t.p[1].z-tra_t.p[0].z};
		Vec3f v2 = {tra_t.p[2].x-tra_t.p[0].x, tra_t.p[2].y-tra_t.p[0].y, tra_t.p[2].z-tra_t.p[0].z};
		CrossProductVec3f(v1, v2, norm);
		float norm_len = sqrtf(norm.x*norm.x+norm.y*norm.y+norm.z*norm.z);
		norm.x /= norm_len; norm.y /= norm_len; norm.z /= norm_len;
		float v_in3[4] = {norm.x+tra_t.p[0].x, norm.y+tra_t.p[0].y, norm.z+tra_t.p[0].z, 1};
		float v_out3[4];
		MultiplyVecMat4x4(v_in3, proj_mat4x4, v_out3);
		float xd1{};
		float xd2{};
		
		Vec3f light_dir = {0.0f, 0.0f, -1.0f};
		float v_len = sqrtf(light_dir.x*light_dir.x+light_dir.y*light_dir.y+light_dir.z*light_dir.z);
		light_dir.x /= v_len; light_dir.y /= v_len; light_dir.z /= v_len;
		float koef = abs(norm.x*light_dir.x+norm.y*light_dir.y+norm.z*light_dir.z);
		int tri_color = MakeColor(255,255*koef,255*koef,255*koef);

		if (norm.x*(tra_t.p[0].x-0/*cam pos*/)+norm.y*(tra_t.p[0].y-0)+norm.z*(tra_t.p[0].z-0/*cam pos*/) <= 0)
		{
			for (int i = 0; i < 3; i++)
			{
				float v_in1[4] = {tra_t.p[i].x, tra_t.p[i].y, tra_t.p[i].z, 1};
				float v_out1[4];
				
				MultiplyVecMat4x4(v_in1, proj_mat4x4, v_out1);
				
				if (i==0)
				{
					xd1 = v_out1[0]/v_out1[3];
					xd2 = v_out1[1]/v_out1[3];
				}
				projected_tri.p[i].x = v_out1[0]/v_out1[3];
				projected_tri.p[i].y = v_out1[1]/v_out1[3];
				projected_tri.p[i].z = v_out1[2];
				/*PlatformDrawLinef(state->bitBuff,
						v_out1[0]/v_out1[3],
						v_out1[1]/v_out1[3],
						v_out2[0]/v_out2[3],
						v_out2[1]/v_out2[3],
						clr);*/
			}
			FillTrianglef(state->bitBuff,
						projected_tri.p[0].x, projected_tri.p[0].y,
						projected_tri.p[1].x, projected_tri.p[1].y,
						projected_tri.p[2].x, projected_tri.p[2].y,
						tri_color);
			/*DrawTrianglef(state->bitBuff,
						projected_tri.p[0].x, projected_tri.p[0].y,
						projected_tri.p[1].x, projected_tri.p[1].y,
						projected_tri.p[2].x, projected_tri.p[2].y,
						MakeColor(255,0,0,0));*/
			/*PlatformDrawLinef(state->bitBuff,
						xd1, 
						xd2,
						v_out3[0]/v_out3[3],
						v_out3[1]/v_out3[3],
						MakeColor(255,255,255,0));*/
		}
		
	}
	
	PlatformDrawLinef(state->bitBuff,
			((0.0f+ox)*state->client_height/state->client_width)/oz, 
			(0.0f+oy)/oz,
			((1.0f+ox)*state->client_height/state->client_width)/oz, 
			(0.0f+oy)/oz,
			MakeColor(255,255,0,0));
	PlatformDrawLinef(state->bitBuff,
			((0.0f+ox)*state->client_height/state->client_width)/oz,
			(0.0f+oy)/oz,
			((0.0f+ox)*state->client_height/state->client_width)/oz,
			(1.0f+oy)/oz,
			MakeColor(255,0,255,0));
	PlatformDrawLinef(state->bitBuff,
			((0.0f+ox)*state->client_height/state->client_width)/(oz+1.0f),
			(0.0f+oy)/(oz+1.0f),
			((0.0f+ox)*state->client_height/state->client_width)/oz,
			(0.0f+oy)/oz,
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