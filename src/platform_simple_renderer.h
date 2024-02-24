/*
platform_simple_renderer.h - (platform independent) core of all smaller projects that draws things on the screen
2023/09/18, peshqa
*/
#pragma once

#include "simple_math.h"
#include "platform_interface.h"

#include <vector>
#include <string>
#include <algorithm>

#include "simple_image.h"

static int Concat(int src1_count, char* src1, int src2_count, char* src2, int dest_count, char* dest)
{
	int dest_idx = 0;
	for (int i = 0; i < src1_count && i < dest_count; i++)
	{
		dest[dest_idx++] = src1[i];
	}
	for (int i = 0; i < src2_count && i < dest_count; i++)
	{
		dest[dest_idx++] = src2[i];
	}
	return 0;
}
// NOTE: for null terminated strings
static void ConcatNT(char* src1, char* src2, char* dest)
{
	while (*dest++ = *src1++);
	dest--;
	while (*dest++ = *src2++);
}

static int ConvertRelToPlain(float rel, int start, int end)
{
	int length = end - start;
	return (int)(length)*rel + start;
}

static int PlatformDrawLine(PlatformBitBuffer *bitBuff, int x1, int y1, int x2, int y2, int color)
{
	// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
	// TODO: add edge cases for horizontal and vertical lines
	int dx = abs(x2 - x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1);
    int sy = y1 < y2 ? 1 : -1;
    int error = dx + dy;
    
    while (true)
	{
        PlatformDrawPixel(bitBuff, x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * error;
        if (e2 >= dy)
		{
            if (x1 == x2) break;
            error = error + dy;
            x1 = x1 + sx;
        }
        if (e2 <= dx)
		{
            if (y1 == y2) break;
            error = error + dx;
            y1 = y1 + sy;
        }
    }
	
	return 0;
}
static int DrawPixelf(PlatformBitBuffer *bitBuff, float x1, float y1, int color)
{
	int end_x = bitBuff->width;
	int end_y = bitBuff->height;
	return PlatformDrawPixel(bitBuff,
			ConvertRelToPlain(x1, 0, end_x),
			ConvertRelToPlain(y1, 0, end_y),
			color);
}
static int PlatformDrawLinef(PlatformBitBuffer *bitBuff, float x1, float y1, float x2, float y2, int color)
{
	int end_x = bitBuff->width;
	int end_y = bitBuff->height;
	return PlatformDrawLine(bitBuff,
			ConvertRelToPlain(x1, 0, end_x),
			ConvertRelToPlain(y1, 0, end_y),
			ConvertRelToPlain(x2, 0, end_x),
			ConvertRelToPlain(y2, 0, end_y),
			color);
}
// util function to draw filled triangle
static int PlatformGetLineXValues(int x1, int y1, int x2, int y2, std::vector<int> &v)
{
	int dx = abs(x2 - x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1);
    int sy = y1 < y2 ? 1 : -1;
    int error = dx + dy;
	
	int last_x = x1-1;
	int last_y = y1;
    
    while (true)
	{
        //PlatformDrawPixel(bitBuff, x1, y1, color);
		if (last_y != y1)
		{
			v.push_back(x1);
		}
		last_y = y1;
		last_x = x1;
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * error;
        if (e2 >= dy)
		{
            if (x1 == x2) break;
            error = error + dy;
            x1 = x1 + sx;
        }
        if (e2 <= dx)
		{
            if (y1 == y2) break;
            error = error + dx;
            y1 = y1 + sy;
        }
    }
	v.push_back(last_x);
	return 0;
}
static int DrawHorizontalLine(PlatformBitBuffer *bitBuff, int x1, int x2, int y, int color)
{
	int tmp;
	if (x2 < x1)
	{
		tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	for (int i = x1; i <= x2; i++)
	{
		PlatformDrawPixel(bitBuff, i, y, color);
	}
	return 0;
}
static int DrawTrianglef(PlatformBitBuffer *bitBuff, float x1, float y1, float x2, float y2, float x3, float y3, int color)
{
	PlatformDrawLinef(bitBuff, x1, y1, x2, y2, color);
	PlatformDrawLinef(bitBuff, x1, y1, x3, y3, color);
	PlatformDrawLinef(bitBuff, x2, y2, x3, y3, color);
	return 0;
}
// Vertical gradient
static int DrawGradientScreen(PlatformBitBuffer *bitBuff, int s_red, int s_green, int s_blue, int e_red, int e_blue, int e_green)
{
	for (int i = 0; i < bitBuff->width; i++)
	{
		for (int j = 0; j < bitBuff->height; j++)
		{
			
			int color = MakeColor(255,
				s_red+  (float)(e_red-s_red)/bitBuff->height*j,
				s_green+(float)(e_green-s_green)/bitBuff->height*j,
				s_blue+ (float)(e_blue-s_blue)/bitBuff->height*j);
			PlatformDrawPixel(bitBuff, i, j, color);
		}
	}
	return 0;
}
static void DrawGradientScreenv(PlatformBitBuffer *bitBuff, Vec3 start_rgb, Vec3 end_rgb)
{
	Vec3 delta_rgb = (end_rgb - start_rgb) / bitBuff->height;
	for (int i = 0; i < bitBuff->width; i++)
	{
		for (int j = 0; j < bitBuff->height; j++)
		{
			
			int color = MakeColor(255,
				(start_rgb.r+delta_rgb.r*j)*255,
				(start_rgb.g+delta_rgb.g*j)*255,
				(start_rgb.b+delta_rgb.b*j)*255);
			PlatformDrawPixel(bitBuff, i, j, color);
		}
	}
}
static int FillTriangle(PlatformBitBuffer *bitBuff, int x1, int y1, int x2, int y2, int x3, int y3, int color)
{
	// Sort the points so that y1 <= y2 <= y3
	int tmp;
	if (y2 < y1)
	{
		tmp = y1;
		y1 = y2;
		y2 = tmp;
		tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	if (y3 < y1)
	{
		tmp = y1;
		y1 = y3;
		y3 = tmp;
		tmp = x1;
		x1 = x3;
		x3 = tmp;
	}
	if (y3 < y2)
	{
		tmp = y2;
		y2 = y3;
		y3 = tmp;
		tmp = x2;
		x2 = x3;
		x3 = tmp;
	}
	
	// Compute the x coordinates of the triangle edges
	std::vector<int> v1;
	std::vector<int> v2;
	PlatformGetLineXValues(x1, y1, x2, y2, v1);
	v1.pop_back();
	PlatformGetLineXValues(x2, y2, x3, y3, v1);
	PlatformGetLineXValues(x1, y1, x3, y3, v2);
	
	int idx = 0;
	for (int i = y1; i <= y3; i++)
	{
		DrawHorizontalLine(bitBuff, v1[idx], v2[idx], i, color);
		idx++;
	}
	
	return 0;
}
static int TextureTriangle(PlatformBitBuffer *bitBuff,
					int x1, int y1, float u1, float v1, float w1,
					int x2, int y2, float u2, float v2, float w2,
					int x3, int y3, float u3, float v3, float w3,
					SimpleImage* img, float* depth_buffer)
{
	if (y2 < y1)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
		
		std::swap(u1, u2);
		std::swap(v1, v2);
		std::swap(w1, w2);
	}
	if (y3 < y1)
	{
		std::swap(x1, x3);
		std::swap(y1, y3);
		
		std::swap(u1, u3);
		std::swap(v1, v3);
		std::swap(w1, w3);
	}
	if (y3 < y2)
	{
		std::swap(x2, x3);
		std::swap(y2, y3);
		
		std::swap(u2, u3);
		std::swap(v2, v3);
		std::swap(w2, w3);
	}

	int dy1 = y2 - y1;
	int dx1 = x2 - x1;
	float dv1 = v2 - v1;
	float du1 = u2 - u1;
	float dw1 = w2 - w1;

	int dy2 = y3 - y1;
	int dx2 = x3 - x1;
	float dv2 = v3 - v1;
	float du2 = u3 - u1;
	float dw2 = w3 - w1;

	float tex_u, tex_v, tex_w;

	float dax_step = 0, dbx_step = 0,
		du1_step = 0, dv1_step = 0,
		du2_step = 0, dv2_step = 0,
		dw1_step=0, dw2_step=0;

	if (dy1)
	{		
		dax_step = dx1 / (float)abs(dy1);
		du1_step = du1 / (float)abs(dy1);
		dv1_step = dv1 / (float)abs(dy1);
		dw1_step = dw1 / (float)abs(dy1);
	}
	if (dy2) 
	{
		dbx_step = dx2 / (float)abs(dy2);
		du2_step = du2 / (float)abs(dy2);
		dv2_step = dv2 / (float)abs(dy2);
		dw2_step = dw2 / (float)abs(dy2);
	}

	
	if (dy1)
	{
		for (int i = y1; i <= y2; i++)
		{
			int ax = x1 + (float)(i - y1) * dax_step;
			int bx = x1 + (float)(i - y1) * dbx_step;

			float tex_su = u1 + (float)(i - y1) * du1_step;
			float tex_sv = v1 + (float)(i - y1) * dv1_step;
			float tex_sw = w1 + (float)(i - y1) * dw1_step;

			float tex_eu = u1 + (float)(i - y1) * du2_step;
			float tex_ev = v1 + (float)(i - y1) * dv2_step;
			float tex_ew = w1 + (float)(i - y1) * dw2_step;

			if (ax > bx)
			{
				std::swap(ax, bx);
				std::swap(tex_su, tex_eu);
				std::swap(tex_sv, tex_ev);
				std::swap(tex_sw, tex_ew);
			}

			tex_u = tex_su;
			tex_v = tex_sv;
			tex_w = tex_sw;

			float tstep = 1.0f / ((float)(bx - ax));
			float t = 0.0f;

			for (int j = ax; j < bx; j++)
			{
				tex_u = (1.0f - t) * tex_su + t * tex_eu;
				tex_v = (1.0f - t) * tex_sv + t * tex_ev;
				tex_w = (1.0f - t) * tex_sw + t * tex_ew;
				
				if (tex_w > depth_buffer[i*bitBuff->width + j])
				{
					int color = SampleTexture(img, tex_u / tex_w, tex_v / tex_w);
					//color = 0xFFFFFFFF;
					PlatformDrawPixel(bitBuff, j, i, color);
					depth_buffer[i*bitBuff->width + j] = tex_w;
				}
				
				t += tstep;
			}

		}
	}

	dy1 = y3 - y2;
	dx1 = x3 - x2;
	dv1 = v3 - v2;
	du1 = u3 - u2;
	dw1 = w3 - w2;

	if (dy1) dax_step = dx1 / (float)abs(dy1);
	if (dy2) dbx_step = dx2 / (float)abs(dy2);

	du1_step = 0, dv1_step = 0;
	if (dy1) du1_step = du1 / (float)abs(dy1);
	if (dy1) dv1_step = dv1 / (float)abs(dy1);
	if (dy1) dw1_step = dw1 / (float)abs(dy1);

	if (dy1)
	{
		for (int i = y2; i <= y3; i++)
		{
			int ax = x2 + (float)(i - y2) * dax_step;
			int bx = x1 + (float)(i - y1) * dbx_step;

			float tex_su = u2 + (float)(i - y2) * du1_step;
			float tex_sv = v2 + (float)(i - y2) * dv1_step;
			float tex_sw = w2 + (float)(i - y2) * dw1_step;

			float tex_eu = u1 + (float)(i - y1) * du2_step;
			float tex_ev = v1 + (float)(i - y1) * dv2_step;
			float tex_ew = w1 + (float)(i - y1) * dw2_step;

			if (ax > bx)
			{
				std::swap(ax, bx);
				std::swap(tex_su, tex_eu);
				std::swap(tex_sv, tex_ev);
				std::swap(tex_sw, tex_ew);
			}

			tex_u = tex_su;
			tex_v = tex_sv;
			tex_w = tex_sw;

			float tstep = 1.0f / ((float)(bx - ax));
			float t = 0.0f;

			for (int j = ax; j < bx; j++)
			{
				tex_u = (1.0f - t) * tex_su + t * tex_eu;
				tex_v = (1.0f - t) * tex_sv + t * tex_ev;
				tex_w = (1.0f - t) * tex_sw + t * tex_ew;
				
				if (tex_w > depth_buffer[i*bitBuff->width + j])
				{
					int color = SampleTexture(img, tex_u / tex_w, tex_v / tex_w);
					//color = 0;
					PlatformDrawPixel(bitBuff, j, i, color);
					depth_buffer[i*bitBuff->width + j] = tex_w;
				}
				t += tstep;
			}
		}	
	}	
	return 0;
}
static int FillTrianglef(PlatformBitBuffer *bitBuff, float x1, float y1, float x2, float y2, float x3, float y3, int color)
{
	int end_x = bitBuff->width;
	int end_y = bitBuff->height;
	return FillTriangle(bitBuff,
			ConvertRelToPlain(x1, 0, end_x),
			ConvertRelToPlain(y1, 0, end_y),
			ConvertRelToPlain(x2, 0, end_x),
			ConvertRelToPlain(y2, 0, end_y),
			ConvertRelToPlain(x3, 0, end_x),
			ConvertRelToPlain(y3, 0, end_y),
			color);
}
static int TextureTrianglef(PlatformBitBuffer *bitBuff,
					float x1, float y1, float u1, float v1, float w1,
					float x2, float y2, float u2, float v2, float w2,
					float x3, float y3, float u3, float v3, float w3,
					SimpleImage* img, float* depth_buffer)
{
	int end_x = bitBuff->width;
	int end_y = bitBuff->height-1;
	return TextureTriangle(bitBuff,
			ConvertRelToPlain(x1, 0, end_x), ConvertRelToPlain(y1, 0, end_y), u1, v1, w1,
			ConvertRelToPlain(x2, 0, end_x), ConvertRelToPlain(y2, 0, end_y), u2, v2, w2,
			ConvertRelToPlain(x3, 0, end_x), ConvertRelToPlain(y3, 0, end_y), u3, v3, w3,
			img, depth_buffer);
}
static int PlatformFillRect(PlatformBitBuffer *bitBuff, int left, int top, int right, int bottom, int color)
{
	for (int y = top; y < bottom; y++)
	{
		for (int x = left; x < right; x++)
		{
			PlatformDrawPixel(bitBuff, x, y, color);
		}
	}
	return 0;
}

static int GetPlatformBitBufferWidth(PlatformBitBuffer *bitBuff)
{
	return bitBuff->width;
}
static int GetPlatformBitBufferHeight(PlatformBitBuffer *bitBuff)
{
	return bitBuff->height;
}

static int FillPlatformBitBuffer(PlatformBitBuffer *bitBuff, int color)
{
	return PlatformFillRect(bitBuff, 0, 0,
		GetPlatformBitBufferWidth(bitBuff),
		GetPlatformBitBufferHeight(bitBuff), color);
}

static int ConvertRelXToX(float rel_x, PlatformBitBuffer *bitBuff)
{
	return (int)(bitBuff->width)*rel_x;
}
static int ConvertRelYToY(float rel_y, PlatformBitBuffer *bitBuff)
{
	return (int)(bitBuff->height)*rel_y;
}
// TODO: get rid of those two below
static int ConvertRelXToXse(float rel_x, int start, int end)
{
	int length = end - start;
	return (int)(length)*rel_x + start;
}
static int ConvertRelYToYse(float rel_y, int start, int end)
{
	int length = end - start;
	return (int)(length)*rel_y + start;
}

static void DrawSimpleEntity(PlatformBitBuffer *bitBuff, Vec2 dim, Vec2 pos, Vec3 color)
{
	int x_min = pos.x*bitBuff->width;
	int y_min = pos.y*bitBuff->height;
	int x_max = (pos.x+dim.x)*bitBuff->width;
	int y_max = (pos.y+dim.y)*bitBuff->height;
	int color32 = MakeColor(255, 255*color.r, 255*color.g, 255*color.b);
	PlatformFillRect(bitBuff, x_min, y_min, x_max, y_max, color32);
}

static void DrawSimpleCircle(PlatformBitBuffer *bitBuff, int radius, Vec2 pos, Vec3 color)
{
	int color32 = MakeColor(255, 255*color.r, 255*color.g, 255*color.b);
	int t1 = radius / 16;
	int t2;
	int x = radius;
	int y = 0;
	while (x >= y)
	{
		/*
		PlatformDrawPixel(bitBuff, pos.x*bitBuff->width-x, pos.y*bitBuff->height-y, color32);
		PlatformDrawPixel(bitBuff, pos.x*bitBuff->width+x, pos.y*bitBuff->height+y, color32);
		PlatformDrawPixel(bitBuff, pos.x*bitBuff->width+y, pos.y*bitBuff->height+x, color32);
		PlatformDrawPixel(bitBuff, pos.x*bitBuff->width-y, pos.y*bitBuff->height-x, color32);
		
		PlatformDrawPixel(bitBuff, pos.x*bitBuff->width+x, pos.y*bitBuff->height-y, color32);
		PlatformDrawPixel(bitBuff, pos.x*bitBuff->width-x, pos.y*bitBuff->height+y, color32);
		PlatformDrawPixel(bitBuff, pos.x*bitBuff->width-y, pos.y*bitBuff->height+x, color32);
		PlatformDrawPixel(bitBuff, pos.x*bitBuff->width+y, pos.y*bitBuff->height-x, color32);*/
		
		DrawHorizontalLine(bitBuff, pos.x*bitBuff->width-x, pos.x*bitBuff->width+x, pos.y*bitBuff->height-y, color32);
		DrawHorizontalLine(bitBuff, pos.x*bitBuff->width-x, pos.x*bitBuff->width+x, pos.y*bitBuff->height+y, color32);
		DrawHorizontalLine(bitBuff, pos.x*bitBuff->width-y, pos.x*bitBuff->width+y, pos.y*bitBuff->height-x, color32);
		DrawHorizontalLine(bitBuff, pos.x*bitBuff->width-y, pos.x*bitBuff->width+y, pos.y*bitBuff->height+x, color32);
		y++;
		t1 += y;
		t2 = t1 - x;
		if (t2 >= 0)
		{
			t1 = t2;
			x--;
		}
	}
	//PlatformFillRect(bitBuff, x_min, y_min, x_max, y_max, color32);
}
static void DrawSimpleCirclef(PlatformBitBuffer *bitBuff, float radiusf, Vec2 pos, Vec3 color)
{
	int radius = radiusf*bitBuff->height;
	int color32 = MakeColor(255, 255*color.r, 255*color.g, 255*color.b);
	int t1 = radius / 16;
	int t2;
	int x = radius;
	int y = 0;
	while (x >= y)
	{
		/*
		PlatformDrawPixel(bitBuff, pos.x*bitBuff->width-x, pos.y*bitBuff->height-y, color32);
		PlatformDrawPixel(bitBuff, pos.x*bitBuff->width+x, pos.y*bitBuff->height+y, color32);
		PlatformDrawPixel(bitBuff, pos.x*bitBuff->width+y, pos.y*bitBuff->height+x, color32);
		PlatformDrawPixel(bitBuff, pos.x*bitBuff->width-y, pos.y*bitBuff->height-x, color32);
		
		PlatformDrawPixel(bitBuff, pos.x*bitBuff->width+x, pos.y*bitBuff->height-y, color32);
		PlatformDrawPixel(bitBuff, pos.x*bitBuff->width-x, pos.y*bitBuff->height+y, color32);
		PlatformDrawPixel(bitBuff, pos.x*bitBuff->width-y, pos.y*bitBuff->height+x, color32);
		PlatformDrawPixel(bitBuff, pos.x*bitBuff->width+y, pos.y*bitBuff->height-x, color32);*/
		
		DrawHorizontalLine(bitBuff, pos.x*bitBuff->width-x, pos.x*bitBuff->width+x, pos.y*bitBuff->height-y, color32);
		DrawHorizontalLine(bitBuff, pos.x*bitBuff->width-x, pos.x*bitBuff->width+x, pos.y*bitBuff->height+y, color32);
		DrawHorizontalLine(bitBuff, pos.x*bitBuff->width-y, pos.x*bitBuff->width+y, pos.y*bitBuff->height-x, color32);
		DrawHorizontalLine(bitBuff, pos.x*bitBuff->width-y, pos.x*bitBuff->width+y, pos.y*bitBuff->height+x, color32);
		y++;
		t1 += y;
		t2 = t1 - x;
		if (t2 >= 0)
		{
			t1 = t2;
			x--;
		}
	}
	//PlatformFillRect(bitBuff, x_min, y_min, x_max, y_max, color32);
}

static int oldLoadFileOBJ(SharedState *s, char *filename_, Vec3 *points, int *points_count, Tri *triangles, int *triangles_count,
				Vec2 *tex_points, int *tex_points_count, Tri *texture_map, int *texture_map_count)
{
	int points_next_free = *points_count;
	int triangles_next_free = *triangles_count;
	int tex_points_next_free = *tex_points_count;
	int texture_map_next_free = *texture_map_count;
	std::string filename = std::string(filename_);
	//std::ifstream file_obj(filename);
	
	if (OpenAssetFileA(s, filename) != 0)
	{
		ASSERT(!"failed to open obj file");
		return 1;
	}
	
	std::string line;
	
	while(ReadAssetLineA(s, line))
	{
		if (line[0] == 'v' && line[1] == ' ')
		{
			// Vertex
			
			std::string vals_str = line.substr(2);
			Vec3 vals;

			std::string val0 = vals_str.substr(0, vals_str.find(' '));
			vals.elem[0] = (float)atof(val0.c_str());

			std::string val1 = vals_str.substr(val0.length() + 1, vals_str.find(' ', val0.length() + 1));
			vals.elem[1] = (float)atof(val1.c_str());
			
			std::string val2 = vals_str.substr(vals_str.find_last_of(' ') + 1);
			vals.elem[2] = (float)atof(val2.c_str());
			
			points[points_next_free++] = vals;
		} else if (line[0] == 'v' && line[1] == 't' && line[2] == ' ') {
			// Texture vertex
			
			std::string vals_str = line.substr(3);
			Vec2 vals;
			
			std::string val0 = vals_str.substr(0, vals_str.find(' '));
			vals.elem[0] = (float)atof(val0.c_str());
			
			std::string val1 = vals_str.substr(vals_str.find_last_of(' ') + 1);
			vals.elem[1] = (float)atof(val1.c_str());
			
			//texture_points.push_back(vals);
			tex_points[tex_points_next_free++] = vals;
		} else if (line[0] == 'v' && line[1] == 'n' && line[2] == ' ') {
			// do nothing
		} else if (line[0] == 'f' && line[1] == ' ') {
			// Face
			
			std::string lineVals = line.substr(2);

			//std::string val0 = lineVals.substr(0, lineVals.find_first_of(' '));
			
			// If the value for this face includes texture and/or 
			// normal, parse them out
			if (lineVals.find('/') >= 0)
			{
				// Get first group of values
				std::string g1 = lineVals.substr(0, lineVals.find(' '));
				
				// Get second group of values
				std::string g2 = line.substr(line.find(' ') + 1);
				g2 = g2.substr(g2.find(' ') + 1);
				//g2 = g2.substr(0, g2.find(' '));
	
				std::string g3 = line.substr(line.find_last_of(' ') + 1);
				
				//int *t_vals = new int[3]{};
				Tri t_vals;
				std::string m1 = g1.substr(g1.find('/')+1, g1.find(' ') - g1.find('/') - 1);
				std::string m2 = g2.substr(g2.find('/')+1, g2.find(' ') - g2.find('/') - 1);
				std::string m3 = g3.substr(g3.find('/')+1, g3.find(' ') - g3.find('/') - 1);
				t_vals.elem[0] = (int)atoi(m1.c_str()) - 1;
				t_vals.elem[1] = (int)atoi(m2.c_str()) - 1;
				t_vals.elem[2] = (int)atoi(m3.c_str()) - 1;
				texture_map[texture_map_next_free++] = (t_vals);
	
				g1 = g1.substr(0, g1.find('/'));
				g2 = g2.substr(0, g2.find('/'));
				g3 = g3.substr(0, g3.find('/'));
				//int *vals = new int[3]{};
				Tri vals;
				vals.elem[0] = (int)atoi(g1.c_str()) - 1;
				vals.elem[1] = (int)atoi(g2.c_str()) - 1;
				vals.elem[2] = (int)atoi(g3.c_str()) - 1;
				triangles[triangles_next_free++] = (vals);
			} else {
				ASSERT(!"unhandled case in LoadFileOBJ - face has no textures");
			}
		}
	}
	
	CloseAssetFile(s);
	*points_count = points_next_free;
	*triangles_count = triangles_next_free;
	*tex_points_count = tex_points_next_free;
	*texture_map_count = texture_map_next_free;
	
	return 0;
}
static char *ParserAdvance(u32 str_size, char *str, u32 *leftover_size, u32 forward)
{
	u32 i = 0;
	for (; i < str_size; i++)
	{
		if (forward == 0)
		{
			break;
		}
		forward--;
	}
	*leftover_size = str_size - i;
	return &str[i];
}
static char *ParseFloat(u32 str_size, char *str, u32 *leftover_size, float *out)
{
	float result = 0.0f;
	//double result = 0.0f;
	u32 i = 0;
	u32 exponent = 0;
	u32 is_expo_neg = 0;
	u32 digits_before_point = 0;
	int before_point = 1;
	int last_i;
	int special_notation = 0;
	int is_negative = 0;
	if (str[0] == '-')
	{
		digits_before_point++;
		is_negative = 1;
		//str++;
		i++;
	}
	for (; i < str_size; i++)
	{
		if ((str[i] == 'e' || str[i] == 'E') && (!special_notation))
		{
			// TODO: add a check for '+' ?
			special_notation = 1;
			last_i = i;
			if (i < str_size - 1 && str[i+1] == '-')
			{
				i++;
				is_expo_neg = 1;
			}
			
			continue;
		}
		if (special_notation)
		{
			if (str[i] < '0' || str[i] > '9')
			{
				break;
			}
			exponent = exponent*10 + str[i] - '0';
		}
		else
		{
			if (str[i] == '.' && before_point)
			{
				before_point = 0;
				continue;
			}
			if (str[i] < '0' || str[i] > '9')
			{
				last_i = i;
				break;
			}
			if (before_point)
			{
				digits_before_point++;
			}
			result = result*10.0f + str[i] - '0';
		}
	}
	if (digits_before_point != last_i)
	{
		float f = 1;
		for (; digits_before_point < last_i-1; digits_before_point++)
		{
			f *= 10;
		}
		float e = 1.0f;
		for (int a = 0; a < exponent; a++)
		{
			e *= 10;
		}
		if (is_expo_neg)
		{
			result /= e;
		} else {
			result *= e;
		}
		result /= f;
	}
	if (is_negative)
	{
		result = -result;
	}
	*out = (float)result;
	*leftover_size = str_size - i;
	return &str[i];
}
static int LoadFileOBJ(SharedState *s, char *filename, Vec3 *points, int *points_count, Tri *triangles, int *triangles_count,
				Vec2 *tex_points, int *tex_points_count, Tri *texture_map, int *texture_map_count)
{
	int points_next_free = *points_count;
	int triangles_next_free = *triangles_count;
	int tex_points_next_free = *tex_points_count;
	int texture_map_next_free = *texture_map_count;
	
	void *memory;
	u32 memory_size = PlatformReadWholeFile(s, filename, memory);
	u32 leftover_size = memory_size;
	
	if (memory_size == 0)
	{
		ASSERT(!"failed to open obj file");
		return 0;
	}
	
	char *line = (char*)memory;

	while (leftover_size > 0)
	{
		if (line[0] == 'v' && line[1] == ' ')
		{
			// Vertex
			line = ParserAdvance(leftover_size, line, &leftover_size, 2);
			Vec3 vals;
			//float test = (float)atof(line);
			line = ParseFloat(leftover_size, line, &leftover_size, &vals.x);
			line = FindNextToken(leftover_size, line, &leftover_size);
			//float test2 = (float)atof(line);
			line = ParseFloat(leftover_size, line, &leftover_size, &vals.y);
			line = FindNextToken(leftover_size, line, &leftover_size);
			//float test3 = (float)atof(line);
			line = ParseFloat(leftover_size, line, &leftover_size, &vals.z);
			//ASSERT(ABS( test - vals.x) < 0.000001f);
			//ASSERT(ABS(test2 - vals.y) < 0.000001f);
			//ASSERT(ABS(test3 - vals.z) < 0.000001f);
			
			points[points_next_free++] = vals;
			//points[points_next_free++] = {test, test2, test3};
		} else if (line[0] == 'v' && line[1] == 't' && line[2] == ' ') {
			// Texture vertex
			line = ParserAdvance(leftover_size, line, &leftover_size, 3);
			Vec2 vals;
			//float test = (float)atof(line);
			line = ParseFloat(leftover_size, line, &leftover_size, &vals.u);
			line = FindNextToken(leftover_size, line, &leftover_size);
			//float test2 = (float)atof(line);
			line = ParseFloat(leftover_size, line, &leftover_size, &vals.v);
			//ASSERT(ABS( test - vals.u) < 0.000001f);
			//ASSERT(ABS(test2 - vals.v) < 0.000001f);
			
			tex_points[tex_points_next_free++] = vals;
			//tex_points[tex_points_next_free++] = {test, test2};
		} else if (line[0] == 'v' && line[1] == 'n' && line[2] == ' ') {
			// do nothing
		} else if (line[0] == 'f' && line[1] == ' ') {
			// Face
			line = ParserAdvance(leftover_size, line, &leftover_size, 2);
			Tri tri;
			Tri tri_tex;
			
			for (int i = 0; i < 3; i++)
			{
				line = FindNextToken(leftover_size, line, &leftover_size);
				//int test = atoi(line);
				line = ParseInteger(leftover_size, line, &leftover_size, (u32*)&tri.elem[i]);
				tri.elem[i]--;
				//ASSERT(test == tri.elem[i]);
				if (line[0] == '/')
				{
					line = ParserAdvance(leftover_size, line, &leftover_size, 1);
					//int test = atoi(line);
					line = ParseInteger(leftover_size, line, &leftover_size, (u32*)&tri_tex.elem[i]);
					tri_tex.elem[i]--;
					//ASSERT(test == tri_tex.elem[i]);
					if (line[0] == '/')
					{
						// Normal information, ingore for now
						u32 xD;
						line = ParserAdvance(leftover_size, line, &leftover_size, 1);
						line = ParseInteger(leftover_size, line, &leftover_size, &xD);
					}
				}
			}
			
			triangles[triangles_next_free++] = tri;
			texture_map[texture_map_next_free++] = tri_tex;
		}
		
		line = FindNextLine(leftover_size, line, &leftover_size);
	}
	
	//ASSERT(line < (char*)memory+memory_size);
	//CloseAssetFile(s);
	PlatformFreeFileMemory(s, memory);
	*points_count = points_next_free;
	*triangles_count = triangles_next_free;
	*tex_points_count = tex_points_next_free;
	*texture_map_count = texture_map_next_free;
	
	return 1;
}
