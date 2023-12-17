/*
project.h - header file contains information about different projects
Every project has a name and function pointers to initiate and update its state.
Projects are provided with pointers to shared app state and can call functions to draw on screen.
2023/07/30, peshqa
*/
#pragma once


#include "platform_simple_renderer.h"
#include "snake_game.h"

#include <list>

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
	
	SimpleImage image;
	
	std::vector<float*> vertices;
	std::vector<int*> triangles;
} ProjectState3DCube;

int InitProject3DCube(SharedState* state)
{
	InitAssetManager(state);
	
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
	
	
	// Vertices of a cube
	float *p0 = new float[3]{0.0f, 0.0f, 0.0f};
	float *p1 = new float[3]{0.0f, 0.0f, 1.0f};
	float *p2 = new float[3]{0.0f, 1.0f, 0.0f};
	float *p3 = new float[3]{0.0f, 1.0f, 1.0f};
	float *p4 = new float[3]{1.0f, 0.0f, 0.0f};
	float *p5 = new float[3]{1.0f, 0.0f, 1.0f};
	float *p6 = new float[3]{1.0f, 1.0f, 0.0f};
	float *p7 = new float[3]{1.0f, 1.0f, 1.0f};
	
	//std::vector<float*> vertices;
	/*p_state->vertices.push_back(p0);
	p_state->vertices.push_back(p1);
	p_state->vertices.push_back(p2);
	p_state->vertices.push_back(p3);
	p_state->vertices.push_back(p4);
	p_state->vertices.push_back(p5);
	p_state->vertices.push_back(p6);
	p_state->vertices.push_back(p7);*/
	
	std::string model_path = state->asset_path + "model.obj";
	std::string texture_path = state->asset_path + "model.ppm";
	LoadFileOBJ(state, model_path, p_state->vertices, p_state->triangles);
	LoadPPMImage(state, texture_path, &p_state->image);
	// Triangles represented as point indices
	int *tri1 =  new int[3]{0, 1, 2};
	int *tri2 =  new int[3]{3, 2, 1};
	int *tri3 =  new int[3]{4, 6, 5};
	int *tri4 =  new int[3]{7, 5, 6};
	int *tri5 =  new int[3]{6, 0, 2};
	int *tri6 =  new int[3]{0, 6, 4};
	int *tri7 =  new int[3]{1, 7, 3};
	int *tri8 =  new int[3]{7, 1, 5};
	int *tri9 =  new int[3]{3, 7, 2};
	int *tri10 = new int[3]{6, 2, 7};
	int *tri11 = new int[3]{1, 0, 5};
	int *tri12 = new int[3]{4, 5, 0};
	/*p_state->triangles.push_back(tri1); // left face
	p_state->triangles.push_back(tri2);
	p_state->triangles.push_back(tri3); // right face
	p_state->triangles.push_back(tri4);
	p_state->triangles.push_back(tri5); // front face
	p_state->triangles.push_back(tri6);
	p_state->triangles.push_back(tri7); // back face
	p_state->triangles.push_back(tri8);
	p_state->triangles.push_back(tri9); // top face
	p_state->triangles.push_back(tri10);
	p_state->triangles.push_back(tri11); // bottom face
	p_state->triangles.push_back(tri12);*/
	
	TerminateAssetManager(state);
	
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
			
			// Camera rotation by mouse
			//game_state->yaw   += 0.005f*(-state->mouse_x+game_state->last_mouse_x);
			//game_state->pitch -= 0.005f*(-state->mouse_y+game_state->last_mouse_y);
			
			// Model rotation by mouse
			game_state->cube_pitch -= 0.005f*(-state->mouse_x+game_state->last_mouse_x);
			game_state->cube_yaw += 0.005f*(-state->mouse_y+game_state->last_mouse_y);
			
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
		game_state->cube_pitch += delta_time*1;
	}
	if (state->input_state[INPUT_UP] == 1)
	{
		game_state->cube_pitch -= delta_time*1;
	}

	float z_near = 0.01f;
	float z_far = 1000.0f;
	float proj_mat4x4[16];
	InitProjectionMat4x4(proj_mat4x4, 90.0f, 1, state->bitBuff->width, state->bitBuff->height, z_near, z_far);
	
	float scale_mat4x4[16];
	//InitScaleMat4x4(scale_mat4x4, 0.3f, 0.3f, 0.3f);
	InitScaleMat4x4(scale_mat4x4, 1, 1, 1);
	
	float combined_mat4x4[16]{};
	float combined2_mat4x4[16]{};
	
	float target[3] = {0.0f, 0.0f, 1.0f};
	Vec3f target_rot_y;
	Vec3f target_rot_yx;
	Vec3f target_final;
	float t_y_rot_mat4x4[16]{};
	float t_x_rot_mat4x4[16]{};
	InitXRotMat4x4(t_y_rot_mat4x4, -game_state->pitch);
	//InitXRotMat4x4(t_y_rot_mat4x4, 0);
	MultiplyVecMat4x4(target, t_y_rot_mat4x4, (float*)&target_rot_y);
	InitYRotMat4x4(t_x_rot_mat4x4, game_state->yaw);
	MultiplyVecMat4x4((float*)&target_rot_y, t_x_rot_mat4x4, (float*)&target_rot_yx);
	
	float move_scale = 1.0f;
	if (state->input_state['W'] == 1)
	{
		game_state->x_offset += target_rot_yx.x*delta_time*move_scale;
		game_state->y_offset += target_rot_yx.y*delta_time*move_scale;
		game_state->z_offset += target_rot_yx.z*delta_time*move_scale;
	}
	if (state->input_state['A'] == 1)
	{
		game_state->z_offset += target_rot_yx.x*delta_time*move_scale;
		//game_state->y_offset += target_rot_yx.y*delta_time*move_scale;
		game_state->x_offset -= target_rot_yx.z*delta_time*move_scale;
	}
	if (state->input_state['S'] == 1)
	{
		game_state->x_offset -= target_rot_yx.x*delta_time*move_scale;
		game_state->y_offset -= target_rot_yx.y*delta_time*move_scale;
		game_state->z_offset -= target_rot_yx.z*delta_time*move_scale;
	}
	if (state->input_state['D'] == 1)
	{
		game_state->z_offset -= target_rot_yx.x*delta_time*move_scale;
		//game_state->y_offset -= target_rot_yx.y*delta_time*move_scale;
		game_state->x_offset += target_rot_yx.z*delta_time*move_scale;
	}
	if (state->input_state[' '] == 1)
	{
		game_state->y_offset += delta_time*2*move_scale;
	}
	if (state->input_state[INPUT_LSHIFT] == 1)
	{
		game_state->y_offset -= delta_time*2*move_scale;
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
	float z_rot_mat4x4[16]{};
	float translate_mat4x4[16];
	
	// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
	InitYRotMat4x4(x_rot_mat4x4, game_state->cube_pitch+
		//state->rot_vec_values[1]
		(2*atan2(sqrt(1+2*(state->rot_vec_values[3]*state->rot_vec_values[1]-state->rot_vec_values[0]*state->rot_vec_values[2])),
		sqrt(1-2*(state->rot_vec_values[3]*state->rot_vec_values[1]-state->rot_vec_values[0]*state->rot_vec_values[2]))) - 3.14159/2)*1
	);
	InitZRotMat4x4(y_rot_mat4x4, game_state->cube_yaw+
		//state->rot_vec_values[0]
		atan2(2*(state->rot_vec_values[3]*state->rot_vec_values[2]+state->rot_vec_values[0]*state->rot_vec_values[1]),
		1-2*(state->rot_vec_values[1]*state->rot_vec_values[1]+state->rot_vec_values[2]*state->rot_vec_values[2]))
	);
	InitXRotMat4x4(z_rot_mat4x4, 
		atan2(2*(state->rot_vec_values[3]*state->rot_vec_values[0]+state->rot_vec_values[1]*state->rot_vec_values[2]),
		1-2*(state->rot_vec_values[0]*state->rot_vec_values[0]+state->rot_vec_values[1]*state->rot_vec_values[1]))
	);
	InitTranslationMat4x4(translate_mat4x4, 0.0f, 0.0f, 1.0f);
	
	MultiplyMats4x4(scale_mat4x4, y_rot_mat4x4, combined_mat4x4);
	MultiplyMats4x4(combined_mat4x4, x_rot_mat4x4, combined2_mat4x4);
	MultiplyMats4x4(combined2_mat4x4, z_rot_mat4x4, combined_mat4x4);
	MultiplyMats4x4(combined_mat4x4, translate_mat4x4, combined2_mat4x4);
	
	FillPlatformBitBuffer(state->bitBuff, MakeColor(255,25,12,6));
	
	int clr = MakeColor(255,255,255,255);
	
	
	
	std::vector<int*> triangles_clipped;
	
	std::vector<int*> lines;
	
	std::vector<float*> vertices_transformed;
	std::vector<float*> vertices_viewed;
	std::vector<float*> vertices_projected;
	
	// Apply world transformations to vertices
	
	for (float *vx: game_state->vertices)
	{
		float v_in1[4] = {vx[0], vx[1], vx[2], 1};
		float v_out1[4];
		
		MultiplyVecMat4x4(v_in1, combined2_mat4x4, v_out1);
		
		float *new_vx = new float[3]{v_out1[0], v_out1[1], v_out1[2]};
		vertices_transformed.push_back(new_vx);
	}
	
	// Apply view matrix
	
	for (float *vx: vertices_transformed)
	{
		float v_in1[4] = {vx[0], vx[1], vx[2], 1};
		float v_out1[4];
		
		MultiplyVecMat4x4(v_in1, look_at_mat4x4, v_out1);
		
		float *new_vx = new float[3]{v_out1[0], v_out1[1], v_out1[2]};
		vertices_viewed.push_back(new_vx);
	}
	
	std::vector<int> tri_colors;
	
	for (int *tri: game_state->triangles)
	{
		int line_color = MakeColor(255, 120, 255, 120);
		float norm[3];
		float v1[3];
		float v2[3];
		VecRaw3fSub(vertices_transformed[tri[1]], vertices_transformed[tri[0]], v1);
		VecRaw3fSub(vertices_transformed[tri[2]], vertices_transformed[tri[0]], v2);
		CrossProductVecRaw3f(v1, v2, norm);
		VecRaw3fNormalize(norm, norm);
		
		/*float v_in3[4] = {norm[0]+vertices_viewed[tri[0]][0], norm[1]+vertices_viewed[tri[0]][1], norm[2]+vertices_viewed[tri[0]][2], 1};
		float v_out3[4];
		MultiplyVecMat4x4(v_in3, proj_mat4x4, v_out3);*/

		if (
			norm[0]*(vertices_transformed[tri[0]][0]-pos.x)+
			norm[1]*(vertices_transformed[tri[0]][1]-pos.y)+
			norm[2]*(vertices_transformed[tri[0]][2]-pos.z) <= 0)
		{
			// Simple directional lighting
			float light_dir[3] = {0.0f, 0.0f, -1.0f};
			VecRaw3fNormalize(light_dir, light_dir);
			float koef = abs(norm[0]*light_dir[0]+norm[1]*light_dir[1]+norm[2]*light_dir[2]);
			if (koef < 0.01f)
				koef = 0.01f;
			int tri_color = MakeColor(255,255*koef,255*koef,255*koef);
			
			// Clipping
			int count_clipped_triangles = 0;
			int tri_clipped[2][3];
			float plane_normal[] = {0.0f, 0.0f, 1.0f};
			float plane_point[] = {0.0f, 0.0f, z_near};
			count_clipped_triangles = ClipAgainstPlane(plane_normal, plane_point, vertices_viewed, tri, tri_clipped[0], tri_clipped[1]);
			//assert(count_clipped_triangles == 1);
			for (int i = 0; i < count_clipped_triangles; i++)
			{
				int* clipped_tri = new int[3]{tri_clipped[i][0], tri_clipped[i][1], tri_clipped[i][2]};
				triangles_clipped.push_back(clipped_tri);
				tri_colors.push_back(tri_color);
			}

			// Draw normals
			/*PlatformDrawLinef(state->bitBuff,
							vertices_projected[tri[0]][0], vertices_projected[tri[0]][1],
							0.5f+v_out3[0]/v_out3[3], 0.5f+v_out3[1]/v_out3[3],
							line_color);*/
		}
		
	}
	
	// Project 3D vertices onto 2D screen
	
	for (float *vx: vertices_viewed)
	{
		float v_in1[4] = {vx[0], vx[1], vx[2], 1};
		float v_out1[4];
		
		MultiplyVecMat4x4(v_in1, proj_mat4x4, v_out1);
		
		float *new_vx = new float[3]{v_out1[0]/v_out1[3]+0.5f, -v_out1[1]/v_out1[3]+0.5f, v_out1[2]};
		vertices_projected.push_back(new_vx);
	}
	
	int color_count = 0;
	for (int *tri: triangles_clipped)
	{
		int tri_color = tri_colors[color_count++];
		int tri_clipped[2][3];
		float plane_normal0[] = {0.0f, 1.0f, 0.0f};
		float  plane_point0[] = {0.0f, 0.0f, 0.0f};
		float plane_normal1[] = {0.0f, -1.0f, 0.0f};
		float  plane_point1[] = {0.0f, 1.0f, 0.0f};
		float plane_normal2[] = {1.0f, 0.0f, 0.0f};
		float  plane_point2[] = {0.0f, 0.0f, 0.0f};
		float plane_normal3[] = {-1.0f, 0.0f, 0.0f};
		float  plane_point3[] = {1.0f, 0.0f, 0.0f};
		
		std::list<int*> tri_batch;
		int* init_tri = new int[3]{tri[0], tri[1], tri[2]};
		tri_batch.push_back(init_tri);
		
		int new_triangles = 1;
		
		for (int plane = 0; plane < 4; plane++)
		{
			int count_clipped_triangles = 0;
			while (new_triangles > 0)
			{
				int *test = tri_batch.front();
				tri_batch.pop_front();
				new_triangles--;
				switch (plane)
				{
					case 0: count_clipped_triangles = 
					ClipAgainstPlane(plane_normal0, plane_point0, vertices_projected, test, tri_clipped[0], tri_clipped[1]); break;
					case 1: count_clipped_triangles = 
					ClipAgainstPlane(plane_normal1, plane_point1, vertices_projected, test, tri_clipped[0], tri_clipped[1]); break;
					case 2: count_clipped_triangles = 
					ClipAgainstPlane(plane_normal2, plane_point2, vertices_projected, test, tri_clipped[0], tri_clipped[1]); break;
					case 3: count_clipped_triangles = 
					ClipAgainstPlane(plane_normal3, plane_point3, vertices_projected, test, tri_clipped[0], tri_clipped[1]); break;
				}
				delete [] test;
				for (int i = 0; i < count_clipped_triangles; i++)
				{
					int* clipped_tri = new int[3]{tri_clipped[i][0], tri_clipped[i][1], tri_clipped[i][2]};
					tri_batch.push_back(clipped_tri);
				}
			}
			new_triangles = tri_batch.size();
		}
		
		for (int *t: tri_batch)
		{
			FillTrianglef(state->bitBuff,
							vertices_projected[t[0]][0], vertices_projected[t[0]][1],
							vertices_projected[t[1]][0], vertices_projected[t[1]][1],
							vertices_projected[t[2]][0], vertices_projected[t[2]][1],
							tri_color);
			/*DrawTrianglef(state->bitBuff,
							vertices_projected[t[0]][0], vertices_projected[t[0]][1],
							vertices_projected[t[1]][0], vertices_projected[t[1]][1],
							vertices_projected[t[2]][0], vertices_projected[t[2]][1],
							clr);*/
			
			delete [] t;
		}
		
	}
	
	// Image drawing
	/*for (int i = 0; i < game_state->image.width; i++)
	{
		for (int j = 0; j < game_state->image.height; j++)
		{
			PlatformDrawPixel(state->bitBuff, i, j, game_state->image.pixels[j*game_state->image.width+i]);
		}
	}*/
	
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
		//DrawPixelf(state->bitBuff, vx[0], vx[1], clr);
		delete [] vx;
	}
	
	/*for (int *vx: lines)
	{
		delete [] vx;
	}*/
	
	/*for (int *vx: triangles)
	{
		delete [] vx;
	}*/
	
	for (int *vx: triangles_clipped)
	{
		delete [] vx;
	}
	
	/*for (float *vx: vertices)
	{
		delete [] vx;
	}*/
	
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