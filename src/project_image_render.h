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
	
	int last_mouse_x;
	int last_mouse_y;
	int was_lmb_down;
	
	SimpleImage image;
	SimpleImage font;
} ProjectStateImageRender;

int InitProjectImageRender(SharedState* state)
{
	ProjectStateImageRender *p_state = new ProjectStateImageRender{};
	p_state->x_offset = 0.0f;
	p_state->y_offset = 0.0f;
	
	p_state->last_mouse_x = 0;
	p_state->last_mouse_y = 0;
	p_state->was_lmb_down = 0;
	
	std::string image_path = state->asset_path + "test.bmp";
	std::string font_path = state->asset_path + "efortless_font_threshold.bmp";
	LoadBMPImage(state, image_path, &p_state->image);
	LoadBMPImage(state, font_path, &p_state->font);
	
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
	
	static float step_x = 0.25f;
	static float step_y = 0.35f;
	
	//FillPlatformBitBuffer(state->bitBuff, MakeColor(255,25,12,6)); // solid color
	DrawGradientScreen(state->bitBuff, 106, 104, 203, 255, 255, 255); // fancy vertical gradient
	
	int min_x = ConvertRelToPlain(ox, 0, state->bitBuff->width);
	int min_y = ConvertRelToPlain(oy, 0, state->bitBuff->height);
	if (step_x > 0 && min_x >= state->bitBuff->width-game_state->image.width)
	{
		step_x = -step_x;
		min_x += step_x * delta_time;
	}
	if (step_x < 0 && min_x < 0)
	{
		step_x = -step_x;
		min_x += step_x * delta_time;
	}
	if (step_y > 0 && min_y >= state->bitBuff->height-game_state->image.height)
	{
		step_y = -step_y;
		min_y += step_y * delta_time;
	}
	if (step_y < 0 && min_y < 0)
	{
		step_y = -step_y;
		min_y += step_y * delta_time;
	}
	DrawImageExceptColor(state->bitBuff, &game_state->image, min_x, min_y, MakeColor(0, 254, 254, 255));
	DrawImageOnlyColor(state->bitBuff, &game_state->font, 0, 0, MakeColor(255, 0, 0, 0));
	DrawBMPFontChar(state->bitBuff, &game_state->font, 100, 100, 'A');
	DrawBMPFontChar(state->bitBuff, &game_state->font, 120, 120, 'Z');
	std::string test_string = "And so, a TEST string is being displayed: 2-2=0";
	DrawBMPFontString(state->bitBuff, &game_state->font, 100, 140, test_string);
	
	test_string = std::to_string(delta_time);
	test_string = "frame time: " + test_string;
	DrawBMPFontString(state->bitBuff, &game_state->font, 100, 170, test_string);
	
	test_string = std::to_string((int)(1.0f / delta_time));
	test_string = "FPS: " + test_string;
	DrawBMPFontString(state->bitBuff, &game_state->font, 100, 190, test_string);
	
	game_state->x_offset += step_x * delta_time;
	game_state->y_offset += step_y * delta_time;
	
	return 0;
}
