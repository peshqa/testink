/*
project_cube3d.h - a simple 3d model renderer
This project features:
- 3d world transformations
- view transformation
- 3d to 2d projection
- back face culling
- texturing
- triangle edge clipping
Project is single threaded and struggles to render high amount of triangles (~10 FPS while rendering ~50k triangles)
2023/12/24, peshqa
*/
#pragma once

#include "platform_simple_renderer.h"

//#include <list>

typedef struct
{
	void *base;
	size_t size;
	size_t used;
} MemoryArena;

#define VEC3_STACK_COUNT 100000
typedef struct
{
	// TODO: do something about this
} Vec3Stack;

typedef struct FreeListNode
{
	Tri data;
	FreeListNode *next;
	FreeListNode *prev;
} FreeListNode;

typedef struct FreeList
{
	FreeListNode *first;
	FreeListNode *last;
	FreeListNode *first_free;
	int size;
} FreeList;

static void *ArenaPush(MemoryArena *arena, size_t size_to_push)
{
	ASSERT(arena->size >= arena->used + size_to_push)
	if (arena->size >= arena->used + size_to_push)
	{
		void *res = (u8 *)arena->base + arena->used;
		arena->used += size_to_push;
		return res;
	}
	return 0;
}

#define ArenaPushArray(arena, elem_count, type) (type*)ArenaPush((arena), (elem_count)*sizeof(type))
#define ArenaPushType(arena, type) (type*)ArenaPush((arena), sizeof(type))

static void ZeroMemory(u8 *p, size_t size_to_clear)
{
	for (size_t i = 0; i < size_to_clear; i++)
	{
		p[i] = 0;
	}
}

void InitFreeList(FreeList *list)
{
	list->first = 0;
	list->last = 0;
	list->first_free = 0;
	list->size = 0;
}
// TODO: add checks in case memory didn't get allocated?
void FreeListPushFront(MemoryArena *arena, FreeList *list, Tri data)
{
	FreeListNode *node;
	if (list->first_free)
	{
		node = list->first_free;
		list->first_free = node->next;
	} else {
		node = ArenaPushType(arena, FreeListNode);
	}
	node->data = data;
	node->prev = 0;
	node->next = list->first;
	list->size++;
	
	if (list->first == 0)
	{
		list->first = list->last = node;
		return;
	}
	
	FreeListNode *first_node = list->first;
	first_node->prev = node;
	
	list->first = node;
}
void FreeListPushBack(MemoryArena *arena, FreeList *list, Tri data)
{
	FreeListNode *node;
	if (list->first_free)
	{
		node = list->first_free;
		list->first_free = node->next;
	} else {
		node = ArenaPushType(arena, FreeListNode);
		//ZeroMemory((u8*)node, 32);
	}
	node->data = data;
	node->prev = list->last;
	node->next = 0;
	list->size++;
	
	if (list->first == 0)
	{
		list->first = list->last = node;
		return;
	}
	
	FreeListNode *last_node = list->last;
	last_node->next = node;
	
	list->last = node;
}
void FreeListPopFront(FreeList *list)
{
	if (list->first == 0)
	{
		return;
	}
	
	FreeListNode *node = list->first;
	
	FreeListNode *first_node = node->next;
	
	list->first = first_node;
	
	if (first_node != 0)
	{
		first_node->prev = 0;
	} else {
		list->last = 0;
	}
	
	node->next = list->first_free;
	list->first_free = node;
	list->size--;
	return;
}
void FreeListPopBack(FreeList *list)
{
	if (list->first == 0)
	{
		return;
	}
	
	FreeListNode *node = list->last;
	
	FreeListNode *last_node = node->prev;
	
	list->last = last_node;
	
	if (last_node != 0)
	{
		last_node->next = 0;
	} else {
		list->first = 0;
	}

	node->next = list->first_free;
	list->first_free = node;
	list->size--;
	return;
}
void FreeListClear(FreeList *list)
{
	if (list->first)
	{
		list->last->next = list->first_free;
		list->first_free = list->first;
		list->first = 0;
		list->last = 0;
		list->size = 0;
	}
}
void CheckFreeList(FreeList *list)
{
	int len = 0;
	FreeListNode *n = list->first;
	while (n)
	{
		n = n->next;
		len++;
	}
	ASSERT(len == list->size);
}

typedef struct
{
	//float x_offset;
	//float y_offset;
	//float z_offset;
	Vec3 offset;
	
	int last_mouse_x;
	int last_mouse_y;
	int was_lmb_down;
	
	float pitch;
	float yaw;
	float roll;
	
	float cube_yaw;
	float cube_pitch;
	
	SimpleImage image;
	
	Tri* tris;
	int tris_count;
	Tri* tris_tex_map;
	int tris_tex_map_count;
	
	Vec3 *verts;
	int verts_count;
	Vec2 *tex_verts;
	int tex_verts_count;
	
	MemoryArena arena;
} ProjectState3DCube;

int InitProject3DCube(SharedState* state)
{
	ProjectState3DCube *p_state = (ProjectState3DCube *)state->project_memory;
	p_state->offset = {0, 0, 0};
	
	p_state->last_mouse_x = 0;
	p_state->last_mouse_y = 0;
	p_state->was_lmb_down = 0;
	
	p_state->pitch = 0.0f;
	p_state->yaw   = 0.0f;
	p_state->roll  = 0.0f;
	
	p_state->cube_yaw   = 0.0f;
	p_state->cube_pitch = 0.0f;
	
	p_state->arena.base = (u8*)(state->project_memory) + sizeof(ProjectState3DCube);
	p_state->arena.size = state->project_memory_size - sizeof(ProjectState3DCube);
	
	p_state->verts = ArenaPushArray(&p_state->arena, VEC3_STACK_COUNT, Vec3);
	p_state->verts_count = 0;
	p_state->tex_verts = ArenaPushArray(&p_state->arena, VEC3_STACK_COUNT, Vec2);
	p_state->tex_verts_count = 0;
	
	p_state->tris = ArenaPushArray(&p_state->arena, VEC3_STACK_COUNT, Tri);
	p_state->tris_count = 0;
	p_state->tris_tex_map = ArenaPushArray(&p_state->arena, VEC3_STACK_COUNT, Tri);
	p_state->tris_tex_map_count = 0;
	
	char model_path[128];
	char texture_path[128];
	ConcatNT(state->asset_path, (char*)"cube.obj", model_path);
	//ConcatNT(state->asset_path, (char*)"test.bmp", texture_path);
	ConcatNT(state->asset_path, (char*)"checkboard.ppm", texture_path);
	LoadFileOBJ(state, model_path, p_state->verts, &p_state->verts_count, p_state->tris, &p_state->tris_count,
				p_state->tex_verts, &p_state->tex_verts_count, p_state->tris_tex_map, &p_state->tris_tex_map_count);
	/*oldLoadFileOBJ(state, model_path, p_state->verts, &p_state->verts_count, p_state->tris, &p_state->tris_count,
				p_state->tex_verts, &p_state->tex_verts_count, p_state->tris_tex_map, &p_state->tris_tex_map_count);*/
	LoadImage(state, texture_path, &p_state->image);

	return 0;
}
//#include <intrin.h>
int UpdateProject3DCube(SharedState* state)
{
	ProjectState3DCube *game_state = (ProjectState3DCube*)(state->project_memory);
	float delta_time = state->delta_time;
	
	// TODO: add memory arena function to track memory used
	size_t memory_used = game_state->arena.used;
	
	uptr verts[] = { (uptr)&game_state->verts, (uptr)&game_state->tex_verts };
	i32 strides[] = { sizeof(float)*3, sizeof(float)*2 };
	ClearCommand(&state->cmdBuff, {.45f, .45f, 0.9f});
	SetVerticesCommand(&state->cmdBuff, verts, strides);
	Vec3 cmd_color = {1.0f, 1.0f, 1.0f};
	for (int i = 0; i < game_state->tris_count; i++)
	{
		//TriangleCommand(&state->cmdBuff, game_state->tris, game_state->tris_tex_map, &game_state->image);
	}
	
	float *depth_buffer;
	depth_buffer = ArenaPushArray(&game_state->arena, state->client_width*state->client_height, float);
	ZeroMemory((u8*)depth_buffer, state->client_width*state->client_height*sizeof(float));
	//ZeroMemory((u8*)game_state->arena.base+game_state->arena.used, game_state->arena.size-game_state->arena.used);
	
	int verts_transformed_next_free = 0;
	Vec3 *verts_transformed = ArenaPushArray(&game_state->arena, VEC3_STACK_COUNT, Vec3);
	int verts_viewed_next_free = 0;
	Vec3 *verts_viewed = ArenaPushArray(&game_state->arena, VEC3_STACK_COUNT, Vec3);
	int verts_projected_next_free = 0;
	Vec3 *verts_projected = ArenaPushArray(&game_state->arena, VEC3_STACK_COUNT, Vec3);
	
	Tri *tris_clipped = ArenaPushArray(&game_state->arena, VEC3_STACK_COUNT, Tri);
	int tris_clipped_next_free = 0;

	Vec3 *clipped_tex_verts = ArenaPushArray(&game_state->arena, VEC3_STACK_COUNT, Vec3);
	int clipped_tex_verts_count = 0;
	
	Tri *tex_map_clipped = ArenaPushArray(&game_state->arena, VEC3_STACK_COUNT, Tri);
	int tex_map_clipped_count = 0;
	
	Vec3 *tri_colors = ArenaPushArray(&game_state->arena, VEC3_STACK_COUNT, Vec3);
	int tri_colors_count = 0;
	
	// Mouse input -> camera rotation control
	
	if (state->is_lmb_down & 1)
	{
		if ((game_state->was_lmb_down & 1) == 0)
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
			game_state->cube_yaw -= 0.005f*(-state->mouse_y+game_state->last_mouse_y);
			
			game_state->last_mouse_x = state->mouse_x;
			game_state->last_mouse_y = state->mouse_y;
		}
		//game_state->x_offset += 1.0f*delta_time;
	} else {
		game_state->was_lmb_down = 0;
	}
	
	if (state->input_state[INPUT_LEFT] & 1)
	{
		game_state->cube_yaw -= delta_time*1;
	}
	if (state->input_state[INPUT_RIGHT] & 1)
	{
		game_state->cube_yaw += delta_time*1;
	}
	if (state->input_state[INPUT_DOWN] & 1)
	{
		game_state->cube_pitch += delta_time*1;
	}
	if (state->input_state[INPUT_UP] & 1)
	{
		game_state->cube_pitch -= delta_time*1;
	}

	//float z_near = 0.01f;
	//float z_far = 1000.0f;
	Mat4 proj_mat4 = MakeProjectionMat4(90.0f, 1, state->bitBuff->width, state->bitBuff->height, 0.01f, 1000.0f);
	
	Vec4 target = {0.0f, 0.0f, 1.0f};
	Vec3 target_final;

	Vec4 target_rot_yx = target * MakeXRotMat4(-game_state->pitch) * MakeYRotMat4(game_state->yaw);
	
	float move_scale = 1.0f;
	if (state->input_state['W'] & 1)
	{
		game_state->offset += target_rot_yx.xyz*delta_time*move_scale;
	}
	if (state->input_state['A'] & 1)
	{
		game_state->offset.z += target_rot_yx.x*delta_time*move_scale;
		//game_state->y_offset += target_rot_yx.y*delta_time*move_scale;
		game_state->offset.x -= target_rot_yx.z*delta_time*move_scale;
	}
	if (state->input_state['S'] & 1)
	{
		game_state->offset -= target_rot_yx.xyz*delta_time*move_scale;
	}
	if (state->input_state['D'] & 1)
	{
		game_state->offset.z -= target_rot_yx.x*delta_time*move_scale;
		//game_state->y_offset -= target_rot_yx.y*delta_time*move_scale;
		game_state->offset.x += target_rot_yx.z*delta_time*move_scale;
	}
	if (state->input_state[' '] & 1)
	{
		game_state->offset.y += delta_time*2*move_scale;
	}
	if (state->input_state[INPUT_LSHIFT] & 1)
	{
		game_state->offset.y -= delta_time*2*move_scale;
	}
	
	Vec3 pos = game_state->offset;
	target_final = pos + target_rot_yx.xyz;
	
	Vec3 up = {0, 1, 0};
	Mat4 look_at_mat4 = QuickInverseMat4(MakePointAtMat4(pos, target_final, up));
	Mat4 combined_mat4 = MakeScaleMat4({1, 1, 1}) * MakeYRotMat4(game_state->cube_pitch) * MakeXRotMat4(game_state->cube_yaw) * MakeTranslationMat4({0.0f, 0.0f, 4.0f});
		
	int clr = MakeColor(255,255,255,255);
	
	// Apply world transformations to vertices
	for (int i = 0; i < game_state->verts_count; i++)
	{
		float *vx = game_state->verts[i].elem;
		Vec4 vec = MakeVec4(vx[0], vx[1], vx[2], 1) * combined_mat4;
		verts_transformed[verts_transformed_next_free++] = vec.xyz;
	}
	
	// Apply view matrix
	for (int i = 0; i < verts_transformed_next_free; i++)
	{
		float *vx = verts_transformed[i].elem;
		Vec4 vec = MakeVec4(vx[0], vx[1], vx[2], 1);
		vec = vec * look_at_mat4;
		
		verts_viewed[verts_viewed_next_free++] = vec.xyz;
	}
	
	for (int i = 0; i < game_state->tex_verts_count; i++)
	{
		float *vx = game_state->tex_verts[i].elem;
		clipped_tex_verts[clipped_tex_verts_count++] = MakeVec3(vx[0], vx[1], 1.0f);
	}
	
	//u64 time_begin_clip = __rdtsc();
	int count = -1;
	for (;count < game_state->tris_count-1;)
	{
		count++;
		int *tri = game_state->tris[count].elem;
		int line_color = MakeColor(255, 120, 255, 120);
		Vec3 v1 = verts_transformed[tri[1]] - verts_transformed[tri[0]];
		Vec3 v2 = verts_transformed[tri[2]] - verts_transformed[tri[0]];
		Vec3 norm = Vec3Normalize(Vec3CrossProd(v1, v2));

		if (Vec3DotProd(norm, verts_transformed[tri[0]] - pos) <= 0)
		{
			// Simple directional lighting
			Vec3 light_dir = Vec3Normalize(MakeVec3(0.0f, 0.0f, -1.0f));
			float koef = fabs(Vec3DotProd(norm, light_dir));
			if (koef < 0.01f)
				koef = 0.01f;
			Vec3 tri_color = {koef, koef, koef};
			
			// Clipping
			int count_clipped_triangles = 0;
			Tri tri_clipped[2];
			Tri tex_clipped[2];
			Vec3 plane_normal = {0.0f, 0.0f, 1.0f};
			Vec3 plane_point = {0.0f, 0.0f, 0.01f};
			
			count_clipped_triangles = ClipAgainstPlane(plane_normal, plane_point, verts_viewed, &verts_viewed_next_free, clipped_tex_verts, &clipped_tex_verts_count,
					tri, game_state->tris_tex_map[count].elem, tri_clipped[0].elem, tex_clipped[0].elem, tri_clipped[1].elem, tex_clipped[1].elem);

			for (int i = 0; i < count_clipped_triangles; i++)
			{
				tris_clipped[tris_clipped_next_free++] = tri_clipped[i];
				tri_colors[tri_colors_count++] = tri_color;
				tex_map_clipped[tex_map_clipped_count++] = tex_clipped[i];
			}

		}
		
	}
	
	// Project 3D vertices onto 2D screen
	int yac = 0;
	while (yac < verts_viewed_next_free)
	{
		float *vx = verts_viewed[yac].elem;
		Vec4 vec = {vx[0], vx[1], vx[2], 1};
		vec = vec * proj_mat4;
		
		//clipped_tex_verts[yac].x /= vec.w;
		//clipped_tex_verts[yac].y /= vec.w;
		//clipped_tex_verts[yac].z = 1.0f / vec.w;
		verts_projected[verts_projected_next_free++] = { vec.x/vec.w+0.5f, vec.y/vec.w+0.5f, vec.z };
		yac++;
	}
	
	FreeList *tri_batch = ArenaPushType(&game_state->arena, FreeList);
	FreeList *tex_batch = ArenaPushType(&game_state->arena, FreeList);
	InitFreeList(tri_batch);
	InitFreeList(tex_batch);
	int color_count = 0;
	for (;color_count < tris_clipped_next_free;)
	{
		int *tri = tris_clipped[color_count].elem;
		int *tex_map = tex_map_clipped[color_count].elem;
		Vec3 tri_color = tri_colors[color_count++];
		Tri tri_clipped[2];
		Tri tex_clipped[2];
		Vec3 plane_normal0 = {0.0f, 1.0f, 0.0f};
		Vec3  plane_point0 = {0.0f, 0.0f, 0.0f};
		Vec3 plane_normal1 = {0.0f, -1.0f, 0.0f};
		Vec3  plane_point1 = {0.0f, 1.0f, 0.0f};
		Vec3 plane_normal2 = {1.0f, 0.0f, 0.0f};
		Vec3  plane_point2 = {0.0f, 0.0f, 0.0f};
		Vec3 plane_normal3 = {-1.0f, 0.0f, 0.0f};
		Vec3  plane_point3 = {1.0f, 0.0f, 0.0f};
		
		FreeListClear(tri_batch);
		FreeListClear(tex_batch);

		FreeListPushBack(&game_state->arena, tri_batch, {tri[0], tri[1], tri[2]});
		FreeListPushBack(&game_state->arena, tex_batch, {tex_map[0], tex_map[1], tex_map[2]});
		
		int new_triangles = 1;
		
		for (int plane = 0; plane < 4; plane++)
		{
			int count_clipped_triangles;
			while (new_triangles > 0)
			{
				Tri test = tri_batch->first->data;
				Tri t_test = tex_batch->first->data;
				
				FreeListPopFront(tri_batch);
				FreeListPopFront(tex_batch);

				new_triangles--;
				switch (plane)
				{
					case 0: count_clipped_triangles = 
						ClipAgainstPlane(plane_normal0, plane_point0, verts_projected, &verts_projected_next_free, clipped_tex_verts, &clipped_tex_verts_count, test.elem, t_test.elem,
						tri_clipped[0].elem, tex_clipped[0].elem,                                                    
						tri_clipped[1].elem, tex_clipped[1].elem); break;                                            
					case 1: count_clipped_triangles =                                                      
						ClipAgainstPlane(plane_normal1, plane_point1, verts_projected, &verts_projected_next_free, clipped_tex_verts, &clipped_tex_verts_count, test.elem, t_test.elem,
						tri_clipped[0].elem, tex_clipped[0].elem,                                                    
						tri_clipped[1].elem, tex_clipped[1].elem); break;                                            
					case 2: count_clipped_triangles =                                                      
						ClipAgainstPlane(plane_normal2, plane_point2, verts_projected, &verts_projected_next_free, clipped_tex_verts, &clipped_tex_verts_count, test.elem, t_test.elem,
						tri_clipped[0].elem, tex_clipped[0].elem,                                                    
						tri_clipped[1].elem, tex_clipped[1].elem); break;                                            
					case 3: count_clipped_triangles =                                                      
						ClipAgainstPlane(plane_normal3, plane_point3, verts_projected, &verts_projected_next_free, clipped_tex_verts, &clipped_tex_verts_count, test.elem, t_test.elem,
						tri_clipped[0].elem, tex_clipped[0].elem,
						tri_clipped[1].elem, tex_clipped[1].elem); break;
				}

				for (int i = 0; i < count_clipped_triangles; i++)
				{
					FreeListPushBack(&game_state->arena, tri_batch, tri_clipped[i]);
					FreeListPushBack(&game_state->arena, tex_batch, tex_clipped[i]);
				}
			}
			new_triangles = tri_batch->size;
			ASSERT(tri_batch->size == tex_batch->size);
		}
		
		FreeListNode *node = tri_batch->first;
		FreeListNode *t_node = tex_batch->first;
		while (node)
		{
			int *tt = t_node->data.elem;
			int *t = node->data.elem;
			/*
			TextureTrianglef(state->bitBuff,
							   verts_projected[t[0]].x,   verts_projected[t[0]].y,
							clipped_tex_verts[tt[0]].x,clipped_tex_verts[tt[0]].y,clipped_tex_verts[tt[0]].z,
							   verts_projected[t[1]].x,   verts_projected[t[1]].y,
							clipped_tex_verts[tt[1]].x,clipped_tex_verts[tt[1]].y, clipped_tex_verts[tt[1]].z,
							   verts_projected[t[2]].x,   verts_projected[t[2]].y,
							clipped_tex_verts[tt[2]].x,clipped_tex_verts[tt[2]].y, clipped_tex_verts[tt[2]].z,
							&game_state->image, depth_buffer);*/
			Vec4 cmd_color = {tri_color.r, tri_color.g, tri_color.b, 1.0f};
			TriangleCommand(&state->cmdBuff, cmd_color, verts_projected, node->data, clipped_tex_verts, t_node->data, &game_state->image);

			node = node->next;
			t_node = t_node->next;
		}
		
	}
	game_state->arena.used = memory_used;
	
	// Sound
	for (int i = 0; i < state->soundBuff.samples_to_fill; i++)
	{
		int16_t value = 0;
		((int16_t*)(state->soundBuff.buffer))[2*i] = value;
		((int16_t*)(state->soundBuff.buffer))[2*i+1] = value;
	}
	
	return 0;
}
