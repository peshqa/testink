/*
project_image_render.h - image rendering test
2023/12/29, peshqa
*/
#pragma once

#include "platform_simple_renderer.h"

typedef struct
{
	float x_offset;
	float y_offset;
	float z_offset;
	
	int last_mouse_x;
	int last_mouse_y;
	int was_lmb_down;
	
	SimpleImage image;
	
} ProjectStateImageRender;

int InitProjectImageRender(SharedState* state)
{
	ProjectStateImageRender *p_state = new ProjectStateImageRender{};
	p_state->x_offset = 0;
	p_state->y_offset = 1;
	p_state->z_offset = 0;
	
	p_state->last_mouse_x = 0;
	p_state->last_mouse_y = 0;
	p_state->was_lmb_down = 0;
	
	std::string image_path = state->asset_path + "test.bmp";
	LoadBMPImage(state, image_path, &p_state->image);
	
	state->project_state = p_state;
	CalculateDeltaTime(state);
	return 0;
}

int UpdateProjectImageRender(SharedState* state)
{
	ProjectStateImageRender *game_state = (ProjectStateImageRender*)(state->project_state);
	float delta_time = CalculateDeltaTime(state);
	
#if 0 // no input for now
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
#endif
	
	float ox = game_state->x_offset;
	float oy = game_state->y_offset;
	float oz = game_state->z_offset;
	
	//FillPlatformBitBuffer(state->bitBuff, MakeColor(255,25,12,6)); // solid color
	DrawGradientScreen(state->bitBuff, 106, 104, 203, 255, 255, 255); // fancy vertical gradient
	
	for (int i = 0; i < game_state->image.width; i++)
	{
		for (int j = 0; j < game_state->image.height; j++)
		{
			PlatformDrawPixel(state->bitBuff, i, j, game_state->image.pixels[game_state->image.width*j+i]);
		}
	}
	
	return 0;
}
