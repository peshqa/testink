/*
project_cube3d.h - a simple 3d model renderer
This project features:
- 3d world transformations
- view transformation
- 3d to 2d projection
- back face culling
- texturing
- triangle edge clipping
Order if triangle vertices is clockwise
Project is single threaded and struggles to render high amount of triangles (~10 FPS while rendering ~50k triangles)
2023/12/24, peshqa
*/
#pragma once

#include "platform_simple_renderer.h"

#include <list>

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
	std::vector<float*> texture_vertices;
	std::vector<int*> tri_tex_map;
} ProjectState3DCube;

int InitProject3DCube(SharedState* state)
{
	//InitAssetManager(state);
	
	ProjectState3DCube *p_state = new ProjectState3DCube{};
	p_state->x_offset = 0;
	p_state->y_offset = 0;
	p_state->z_offset = 0;
	
	p_state->last_mouse_x = 0;
	p_state->last_mouse_y = 0;
	p_state->was_lmb_down = 0;
	
	p_state->pitch = 0.0f;
	p_state->yaw   = 0.0f;
	p_state->roll  = 0.0f;
	
	p_state->cube_yaw   = 0.0f;
	p_state->cube_pitch = 0.0f;
	
	std::string model_name = "cube";
	std::string model_path = state->asset_path + model_name + ".obj";
	std::string texture_path = state->asset_path + model_name + ".ppm";
	LoadFileOBJ(state, model_path, p_state->vertices, p_state->triangles, p_state->texture_vertices, p_state->tri_tex_map);
	LoadPPMImage(state, texture_path, &p_state->image);
	
	state->project_state = p_state;
	return 0;
}

int UpdateProject3DCube(SharedState* state)
{
	ProjectState3DCube *game_state = (ProjectState3DCube*)(state->project_state);
	float delta_time = state->delta_time;
	static float cam_rot = 0.0f;
	
	float *depth_buffer = new float[state->client_width*state->client_height];
	
	// Mouse input -> camera rotation control
	
	if (state->is_lmb_down == 1)
	{
		if (game_state->was_lmb_down == 0)
		{
			game_state->was_lmb_down = 1;
			game_state->last_mouse_x = state->mouse_x;
			game_state->last_mouse_y = state->mouse_y;
		} else {
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
		(2*atan2(sqrt(1+2*(state->rot_vec_values[3]*state->rot_vec_values[1]-state->rot_vec_values[0]*state->rot_vec_values[2])),
		sqrt(1-2*(state->rot_vec_values[3]*state->rot_vec_values[1]-state->rot_vec_values[0]*state->rot_vec_values[2]))) - 3.14159/2)*1
	);
	InitZRotMat4x4(y_rot_mat4x4, game_state->cube_yaw+
		atan2(2*(state->rot_vec_values[3]*state->rot_vec_values[2]+state->rot_vec_values[0]*state->rot_vec_values[1]),
		1-2*(state->rot_vec_values[1]*state->rot_vec_values[1]+state->rot_vec_values[2]*state->rot_vec_values[2]))
	);
	InitXRotMat4x4(z_rot_mat4x4, 
		atan2(2*(state->rot_vec_values[3]*state->rot_vec_values[0]+state->rot_vec_values[1]*state->rot_vec_values[2]),
		1-2*(state->rot_vec_values[0]*state->rot_vec_values[0]+state->rot_vec_values[1]*state->rot_vec_values[1]))
	);
	InitTranslationMat4x4(translate_mat4x4, 0.0f, 0.0f, 4.0f);
	
	MultiplyMats4x4(scale_mat4x4, y_rot_mat4x4, combined_mat4x4);
	MultiplyMats4x4(combined_mat4x4, x_rot_mat4x4, combined2_mat4x4);
	MultiplyMats4x4(combined2_mat4x4, z_rot_mat4x4, combined_mat4x4);
	MultiplyMats4x4(combined_mat4x4, translate_mat4x4, combined2_mat4x4);
	
	//FillPlatformBitBuffer(state->bitBuff, MakeColor(255,25,12,6)); // solid color
	DrawGradientScreen(state->bitBuff, 106, 104, 203, 255, 255, 255); // fancy vertical gradient
	
	int clr = MakeColor(255,255,255,255);
	
	std::vector<int*> triangles_clipped;
	std::vector<int*> tex_map_clipped;
	
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
	
	std::vector<float*> clipped_tex_verts;
	for (float *vx: game_state->texture_vertices)
	{
		float *new_vx = new float[3]{vx[0], vx[1], 1.0f};
		clipped_tex_verts.push_back(new_vx);
	}
	
	int count = -1;
	for (int *tri: game_state->triangles)
	{
		count++;
		int line_color = MakeColor(255, 120, 255, 120);
		float norm[3];
		float v1[3];
		float v2[3];
		VecRaw3fSub(vertices_transformed[tri[1]], vertices_transformed[tri[0]], v1);
		VecRaw3fSub(vertices_transformed[tri[2]], vertices_transformed[tri[0]], v2);
		CrossProductVecRaw3f(v1, v2, norm);
		VecRaw3fNormalize(norm, norm);

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
			int tex_clipped[2][3];
			float plane_normal[] = {0.0f, 0.0f, 1.0f};
			float plane_point[] = {0.0f, 0.0f, z_near};
			count_clipped_triangles = ClipAgainstPlane(plane_normal, plane_point, vertices_viewed, clipped_tex_verts,
					tri, game_state->tri_tex_map[count], tri_clipped[0], tex_clipped[0], tri_clipped[1], tex_clipped[1]);

			for (int i = 0; i < count_clipped_triangles; i++)
			{
				int* clipped_tri = new int[3]{tri_clipped[i][0], tri_clipped[i][1], tri_clipped[i][2]};
				triangles_clipped.push_back(clipped_tri);
				tri_colors.push_back(tri_color);
				
				int* clipped_tex = new int[3]{tex_clipped[i][0], tex_clipped[i][1], tex_clipped[i][2]};
				tex_map_clipped.push_back(clipped_tex);
			}

		}
		
	}
	
	// Project 3D vertices onto 2D screen
	int yac = 0;
	for (float *vx: vertices_viewed)
	{
		float v_in1[4] = {vx[0], vx[1], vx[2], 1};
		float v_out1[4];
		
		MultiplyVecMat4x4(v_in1, proj_mat4x4, v_out1);
		//assert(0);
		float *new_vx = new float[3]{v_out1[0]/v_out1[3]+0.5f, -v_out1[1]/v_out1[3]+0.5f, v_out1[2]};
		clipped_tex_verts[yac][0] /= v_out1[3];
		clipped_tex_verts[yac][1] /= v_out1[3];
		clipped_tex_verts[yac][2] = 1.0f / v_out1[3];
		yac++;
		vertices_projected.push_back(new_vx);
	}
	
	int color_count = 0;
	for (int *tri: triangles_clipped)
	{
		int *tex_map = tex_map_clipped[color_count];
		int tri_color = tri_colors[color_count++];
		int tri_clipped[2][3];
		int tex_clipped[2][3];
		float plane_normal0[] = {0.0f, 1.0f, 0.0f};
		float  plane_point0[] = {0.0f, 0.0f, 0.0f};
		float plane_normal1[] = {0.0f, -1.0f, 0.0f};
		float  plane_point1[] = {0.0f, 1.0f, 0.0f};
		float plane_normal2[] = {1.0f, 0.0f, 0.0f};
		float  plane_point2[] = {0.0f, 0.0f, 0.0f};
		float plane_normal3[] = {-1.0f, 0.0f, 0.0f};
		float  plane_point3[] = {1.0f, 0.0f, 0.0f};
		
		std::list<int*> tri_batch;
		std::list<int*> tex_batch;
		int* init_tri = new int[3]{tri[0], tri[1], tri[2]};
		int* init_tex = new int[3]{tex_map[0], tex_map[1], tex_map[2]};
		tri_batch.push_back(init_tri);
		tex_batch.push_back(init_tex);
		
		int new_triangles = 1;
		
		for (int plane = 0; plane < 4; plane++)
		{
			int count_clipped_triangles = 0;
			while (new_triangles > 0)
			{
				int *test = tri_batch.front();
				int *t_test = tex_batch.front();
				tri_batch.pop_front();
				tex_batch.pop_front();
				new_triangles--;
				switch (plane)
				{
					case 0: count_clipped_triangles = 
						ClipAgainstPlane(plane_normal0, plane_point0, vertices_projected, clipped_tex_verts, test, t_test,
						tri_clipped[0], tex_clipped[0],                                                    
						tri_clipped[1], tex_clipped[1]); break;                                            
					case 1: count_clipped_triangles =                                                      
						ClipAgainstPlane(plane_normal1, plane_point1, vertices_projected, clipped_tex_verts, test, t_test,
						tri_clipped[0], tex_clipped[0],                                                    
						tri_clipped[1], tex_clipped[1]); break;                                            
					case 2: count_clipped_triangles =                                                      
						ClipAgainstPlane(plane_normal2, plane_point2, vertices_projected, clipped_tex_verts, test, t_test,
						tri_clipped[0], tex_clipped[0],                                                    
						tri_clipped[1], tex_clipped[1]); break;                                            
					case 3: count_clipped_triangles =                                                      
						ClipAgainstPlane(plane_normal3, plane_point3, vertices_projected, clipped_tex_verts, test, t_test,
						tri_clipped[0], tex_clipped[0],
						tri_clipped[1], tex_clipped[1]); break;
				}
				delete [] test;
				delete [] t_test;
				for (int i = 0; i < count_clipped_triangles; i++)
				{
					int* clipped_tri = new int[3]{tri_clipped[i][0], tri_clipped[i][1], tri_clipped[i][2]};
					tri_batch.push_back(clipped_tri);
					
					int* clipped_tex = new int[3]{tex_clipped[i][0], tex_clipped[i][1], tex_clipped[i][2]};
					tex_batch.push_back(clipped_tex);
				}
			}
			new_triangles = tri_batch.size();
		}
		
		auto iter = tex_batch.begin();
		
		for (int *t: tri_batch)
		{
			/*FillTrianglef(state->bitBuff,
							vertices_projected[t[0]][0], vertices_projected[t[0]][1],
							vertices_projected[t[1]][0], vertices_projected[t[1]][1],
							vertices_projected[t[2]][0], vertices_projected[t[2]][1],
							tri_color);*/
			/*DrawTrianglef(state->bitBuff,
							vertices_projected[t[0]][0], vertices_projected[t[0]][1],
							vertices_projected[t[1]][0], vertices_projected[t[1]][1],
							vertices_projected[t[2]][0], vertices_projected[t[2]][1],
							clr);*/
			int *tt = *iter;

			TextureTrianglef(state->bitBuff,
							vertices_projected[t[0]][0], vertices_projected[t[0]][1],
							clipped_tex_verts[tt[0]][0], clipped_tex_verts[tt[0]][1], clipped_tex_verts[tt[0]][2],
							vertices_projected[t[1]][0], vertices_projected[t[1]][1],
							clipped_tex_verts[tt[1]][0], clipped_tex_verts[tt[1]][1], clipped_tex_verts[tt[1]][2],
							vertices_projected[t[2]][0], vertices_projected[t[2]][1],
							clipped_tex_verts[tt[2]][0], clipped_tex_verts[tt[2]][1], clipped_tex_verts[tt[2]][2],
							&game_state->image, depth_buffer);

			std::advance(iter, 1);
			
			delete [] t;
			delete [] tt;
		}
		
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
	
	for (int *vx: triangles_clipped)
	{
		delete [] vx;
	}
	
	for (int *vx: tex_map_clipped)
	{
		delete [] vx;
	}
	
	for (float *vx: clipped_tex_verts)
	{
		delete [] vx;
	}

	delete [] depth_buffer;
	
	return 0;
}
