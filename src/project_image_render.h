/*
project_image_render.h - image rendering test
2023/12/29, peshqa
*/
#pragma once

#include "platform_simple_renderer.h"

typedef struct
{
	Vec2 dim;
	Vec2 pos;
	Vec2 vel;
	Vec2 accel;
	Vec3 color;
} SimpleEntity;

typedef struct
{
	float x_offset;
	float y_offset;
	
	int last_mouse_x;
	int last_mouse_y;
	int was_lmb_down;
	
	SimpleImage image;
	SimpleImage font;
	
	SimpleEntity ent;
	
	float wave_period;
	float t_sin;
	
	float step_x;
	float step_y;
	
	float offset;
} ProjectStateImageRender;

int InitProjectImageRender(SharedState* state)
{
	ProjectStateImageRender *p_state = (ProjectStateImageRender *)state->project_memory;
	p_state->offset = 0.0f;
	
	p_state->x_offset = 0.0f;
	p_state->y_offset = 0.0f;
	
	p_state->last_mouse_x = 0;
	p_state->last_mouse_y = 0;
	p_state->was_lmb_down = 0;
	
	p_state->ent.dim = {0.25f, 0.25f};
	p_state->ent.pos = {0.5f, 0.5f};
	p_state->ent.color = {0.85f, 0.75f, 0.1f};
	
	p_state->wave_period = 220.0f;
	p_state->t_sin = 0.0f;
	
	p_state->step_x = 0.25f;
	p_state->step_y = 0.35f;
	
	char image_path[128];
	char font_path[128];
	ConcatNT(state->asset_path, (char*)"test.bmp", image_path);
	ConcatNT(state->asset_path, (char*)"efortless_font_threshold.bmp", font_path);
	
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
	
	//FillPlatformBitBuffer(state->bitBuff, MakeColor(255,50,24,6)); // solid color
	DrawGradientScreenv(state->bitBuff, {.45f, .45f, 0.9f}, {1.0f, 1.0f, 1.0f}); // fancy vertical gradient
	
	game_state->ent.accel = {};
	if (((state->input_state['W'] & 0b1) == 0b1))
	{
		game_state->ent.accel.y = 1.0f;
	}
	if (((state->input_state['S'] & 0b1) == 0b1))
	{
		game_state->ent.accel.y = -1.0f;
	}
	if (((state->input_state['A'] & 0b1) == 0b1))
	{
		game_state->ent.accel.x = -1.0f;
	}
	if (((state->input_state['D'] & 0b1) == 0b1))
	{
		game_state->ent.accel.x = 1.0f;
	}
	game_state->ent.pos = game_state->ent.accel*0.5f*delta_time*delta_time + game_state->ent.vel*delta_time + game_state->ent.pos;
	game_state->ent.vel = game_state->ent.accel*delta_time + game_state->ent.vel;
	
	//game_state->ent.vel = game_state->ent.vel * (1.0f - 660.99f * delta_time);
	
	
	game_state->offset += 0.25*delta_time;
	if (game_state->offset > 0.2f)
		game_state->offset -= 0.2f;
	for (int i = 4; i >= 0; i--)
	{
		DrawSimpleCirclef(state->bitBuff, 0.10f+.2f*i+game_state->offset, {0.5f, 0.5f}, {});
		DrawSimpleCirclef(state->bitBuff, 0.0f+.2f*i+game_state->offset, {0.5f, 0.5f}, {0.5f, 0.5f, 1});
	}
	DrawSimpleEntity(state->bitBuff, game_state->ent.dim, game_state->ent.pos, game_state->ent.color);
	/*int min_x = ConvertRelToPlain(ox, 0, state->bitBuff->width);
	int min_y = ConvertRelToPlain(oy, 0, state->bitBuff->height);
	if (game_state->step_x > 0 && min_x >= state->bitBuff->width-game_state->image.width)
	{
		game_state->step_x = -game_state->step_x;
		min_x += game_state->step_x * delta_time;
	}
	if (game_state->step_x < 0 && min_x < 0)
	{
		game_state->step_x = -game_state->step_x;
		min_x += game_state->step_x * delta_time;
	}
	if (game_state->step_y > 0 && min_y >= state->bitBuff->height-game_state->image.height)
	{
		game_state->step_y = -game_state->step_y;
		min_y += game_state->step_y * delta_time;
	}
	if (game_state->step_y < 0 && min_y < 0)
	{
		game_state->step_y = -game_state->step_y;
		min_y += game_state->step_y * delta_time;
	}
	DrawImageExceptColor(state->bitBuff, &game_state->image, min_x, min_y, MakeColor(0, 254, 254, 255));
	
	
	game_state->x_offset += game_state->step_x * delta_time;
	game_state->y_offset += game_state->step_y * delta_time;*/
	
	//std::string test_string = std::to_string(delta_time);
	/*std::string test_string = "";
	test_string = "ft: " + test_string;
	DrawBMPFontString(state->bitBuff, &game_state->font, 100, 170, test_string);
	
	//test_string = std::to_string((int)(1.0f / delta_time));
	test_string = "FPS: " + test_string;
	DrawBMPFontString(state->bitBuff, &game_state->font, 100, 190, test_string);*/
	
	
	if (((state->input_state['W'] & 0b11) == 0b01))
	{
		game_state->wave_period += 100.0f*delta_time;
	}
	if (state->input_state['S'] & 1)
	{
		game_state->wave_period -= 100.0f*delta_time;
	}
	
	// Sound
	for (int i = 0; i < state->soundBuff.samples_to_fill; i++)
	{
		float s = sinf(game_state->t_sin);
		//int16_t value = s*0xFFFF*0.1f;
		int16_t value = 0;
		((int16_t*)(state->soundBuff.buffer))[2*i] = value;
		((int16_t*)(state->soundBuff.buffer))[2*i+1] = value;
		game_state->t_sin += 2.0f * PI32 * (1.0f / game_state->wave_period);
		if (game_state->t_sin > 2.0f * PI32)
		{
			game_state->t_sin -= 2.0f * PI32; // have to do this so that we don't run out of float precision
		}
		//DrawPixelf(state->bitBuff, (float)i/8000.0f, s/4+0.5f, 0xFF000000);
	}
	
	return 0;
}
