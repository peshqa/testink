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
	
	uint32_t running_sample_index;
	float last_sin;
} ProjectStateImageRender;

int InitProjectImageRender(SharedState* state)
{
	ProjectStateImageRender *p_state = (ProjectStateImageRender *)state->project_memory;
	p_state->x_offset = 0.0f;
	p_state->y_offset = 0.0f;
	
	p_state->last_mouse_x = 0;
	p_state->last_mouse_y = 0;
	p_state->was_lmb_down = 0;
	
	char image_path[128];
	char font_path[128];
	ConcatNT(state->asset_path, "test.bmp", image_path);
	ConcatNT(state->asset_path, "efortless_font_threshold.bmp", font_path);
	
	LoadBMPImage(state, image_path, &p_state->image);
	LoadBMPImage(state, font_path, &p_state->font);
	
	return 0;
}

int UpdateProjectImageRender(SharedState* state)
{
	ProjectStateImageRender *game_state = (ProjectStateImageRender*)(state->project_memory);
	float delta_time = state->delta_time;
	
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
	std::string test_string = "TEST string";
	DrawBMPFontString(state->bitBuff, &game_state->font, 100, 140, test_string);
	
	test_string = std::to_string(delta_time);
	test_string = "ft: " + test_string;
	DrawBMPFontString(state->bitBuff, &game_state->font, 100, 170, test_string);
	
	test_string = std::to_string((int)(1.0f / delta_time));
	test_string = "FPS: " + test_string;
	DrawBMPFontString(state->bitBuff, &game_state->font, 100, 190, test_string);
	
	game_state->x_offset += step_x * delta_time;
	game_state->y_offset += step_y * delta_time;
	
	static float wave_period = 220.0f;
	static float t_sin = 0;
	
	if (state->input_state['W'])
	{
		wave_period += 100.0f*delta_time;
	}
	if (state->input_state['S'])
	{
		wave_period -= 100.0f*delta_time;
	}
	
	// Sound
	for (int i = 0; i < state->soundBuff.samples_to_fill; i++)
	{
		float s = sinf(t_sin);
		int16_t value = s*0xFFFF*0.1f;
		((int16_t*)(state->soundBuff.buffer))[2*i] = value;
		((int16_t*)(state->soundBuff.buffer))[2*i+1] = value;
		t_sin += 2.0f * PI32 * (1.0f / wave_period);
		if (t_sin > 2.0f * PI32)
		{
			t_sin -= 2.0f * PI32; // have to do this so that we don't run out of float precision
		}
		DrawPixelf(state->bitBuff, (float)i/8000.0f, s/4+0.5f, 0xFF000000);
	}
	
	return 0;
}
