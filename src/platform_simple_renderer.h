/*
platform_simple_renderer.h - (platform independent) core of all smaller projects that draws things on the screen
2023/09/18, peshqa
*/
#pragma once

#include "simple_math.h"
#include "platform_interface.h"
#include "simple_image.h"

// TODO: delete this?
typedef struct
{
	SimpleImage img;
	// TODO: do we care?
	//u32 filtering;
	//u32 wrapping;
} SimpleTexture;

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
	while ((*dest++ = *src1++));
	dest--;
	while ((*dest++ = *src2++));
}

static int ConvertRelToPlain(float rel, int start, int end)
{
	int length = end - start;
	return (int)(length)*rel + start;
}

static int PlatformDrawPixel(PlatformBitBuffer *bitBuff, int x, int y, int color)
{
	if (x < 0 || x >= bitBuff->width || y < 0 || y >= bitBuff->height)
	{
		return -1;
	}
	if (bitBuff->is_top_to_bottom)
	{
		y = bitBuff->height - y - 1;
	}
	((int*)(bitBuff->bits))[y*bitBuff->stride+x] = color;
	return 0;
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
// Vertical gradient, probably very inefficient
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
static void Swap(int *one, int *two)
{
	int temp = *one;
	*one = *two;
	*two = temp;
}
static void Swap(float *one, float *two)
{
	float temp = *one;
	*one = *two;
	*two = temp;
}
static void Swap(Vec2 *one, Vec2 *two)
{
	Vec2 temp = *one;
	*one = *two;
	*two = temp;
}
static void Swap(Vec3 *one, Vec3 *two)
{
	Vec3 temp = *one;
	*one = *two;
	*two = temp;
}

float edge_function(Vec2 a, Vec2 b, Vec2 c)
{
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

static void newTextureTriangle(PlatformBitBuffer *bitBuff,
					Vec3 v0, Vec3 v1, Vec3 v2,
					Vec3 tv0, Vec3 tv1, Vec3 tv2,
					SimpleImage *img, float *depth_buffer, Vec4 color)
{
#if 1
	v0.x = v0.x*bitBuff->width;
	v0.y = v0.y*bitBuff->height;
	v1.x = v1.x*bitBuff->width;
	v1.y = v1.y*bitBuff->height;
	v2.x = v2.x*bitBuff->width;
	v2.y = v2.y*bitBuff->height;
#else
	v0.x = v0.x*(bitBuff->width -1);
	v0.y = v0.y*(bitBuff->height-1);
	v1.x = v1.x*(bitBuff->width -1);
	v1.y = v1.y*(bitBuff->height-1);
	v2.x = v2.x*(bitBuff->width -1);
	v2.y = v2.y*(bitBuff->height-1);
#endif
    v0.z = 1 / v0.z;
	v1.z = 1 / v1.z;
	v2.z = 1 / v2.z;
	
	tv0 *= v0.z;
    tv1 *= v1.z;
    tv2 *= v2.z;
	
	float area = edge_function(v0.xy, v1.xy, v2.xy);
	
	float test = MAX(0.0f, MIN(MIN(v0.y, v1.y), v2.y));
	float test2 = MAX(0.0f, MIN(MIN(v0.x, v1.x), v2.x));
	
	float test3 = MIN(bitBuff->height, MAX(MAX(v0.y, v1.y), v2.y));
	float test4 = MIN(bitBuff->width, MAX(MAX(v0.x, v1.x), v2.x));
	
	for (u32 j = (u32)test; j < test3; ++j)
	{
        for (u32 i = (u32)test2; i < test4; ++i)
		{
            Vec2 p = {i + 0.5f, j + 0.5f};
            float w0 = edge_function(v1.xy, v2.xy, p);
            float w1 = edge_function(v2.xy, v0.xy, p);
            float w2 = edge_function(v0.xy, v1.xy, p);
            if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 < 0 && w1 < 0 && w2 < 0))
			//if (w0 >= 0 && w1 >= 0 && w2 >= 0)
			{
				w0 /= area;
				w1 /= area;
				w2 /= area;
				
				float oneOverZ = v0.z * w0 + v1.z * w1 + v2.z * w2;
				float z = 1 / oneOverZ;
				if (depth_buffer && z < depth_buffer[j*bitBuff->width + i])
				{
					float s = w0 * tv0.u + w1 * tv1.u + w2 * tv2.u;
					float t = w0 * tv0.v + w1 * tv1.v + w2 * tv2.v;
					s *= z;
					t *= z;
					
					//int color_ = MakeColor(1*255, r*255, g*255, b*255);
					int color_ = SampleTexture(img, s, t);
					Vec4 unpacked = ColorIntToVec4(color_);
					color_ = MakeColor(unpacked.a*color.a*255, unpacked.r*color.r*255, unpacked.g*color.g*255, unpacked.b*color.b*255);
					PlatformDrawPixel(bitBuff, i, j, color_);
					depth_buffer[j*bitBuff->width + i] = z;
				} else {
					// ...
				}
               
            }
        }
    }
	//ASSERT(0);
}
static int TextureTriangle(PlatformBitBuffer *bitBuff,
					int x1, int y1, float u1, float v1, float w1,
					int x2, int y2, float u2, float v2, float w2,
					int x3, int y3, float u3, float v3, float w3,
					SimpleImage* img, float* depth_buffer, Vec4 color)
{
	int color_ = MakeColor(color.a*255, color.r*255, color.g*255, color.b*255);
	//w1 = w2 = w3 = 1.0f;
	if (y2 < y1)
	{
		Swap(&x1, &x2);
		Swap(&y1, &y2);
		
		Swap(&u1, &u2);
		Swap(&v1, &v2);
		Swap(&w1, &w2);
	}
	if (y3 < y1)
	{
		Swap(&x1, &x3);
		Swap(&y1, &y3);
		
		Swap(&u1, &u3);
		Swap(&v1, &v3);
		Swap(&w1, &w3);
	}
	if (y3 < y2)
	{
		Swap(&x2, &x3);
		Swap(&y2, &y3);

		Swap(&u2, &u3);
		Swap(&v2, &v3);
		Swap(&w2, &w3);
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
				Swap(&ax, &bx);
				Swap(&tex_su, &tex_eu);
				Swap(&tex_sv, &tex_ev);
				Swap(&tex_sw, &tex_ew);
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
				
				if (depth_buffer && tex_w > depth_buffer[i*bitBuff->width + j])
				{
					int color_ = SampleTexture(img, tex_u / tex_w, tex_v / tex_w);
					//int color_ = MakeColor(color.a, color.r, color.g, color.b);
					PlatformDrawPixel(bitBuff, j, i, color_);
					depth_buffer[i*bitBuff->width + j] = tex_w;
				} else {
					//PlatformDrawPixel(bitBuff, j, i, 0xFFFFFFFF);
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
				Swap(&ax, &bx);
				Swap(&tex_su, &tex_eu);
				Swap(&tex_sv, &tex_ev);
				Swap(&tex_sw, &tex_ew);
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
				
				if (depth_buffer && tex_w > depth_buffer[i*bitBuff->width + j])
				{
					int color_ = SampleTexture(img, tex_u / tex_w, tex_v / tex_w);
					//int color_ = MakeColor(color.a, color.r, color.g, color.b);
					PlatformDrawPixel(bitBuff, j, i, color_);
					depth_buffer[i*bitBuff->width + j] = tex_w;
				} else {
					//PlatformDrawPixel(bitBuff, j, i, 0xFFFFFFFF);
				}
				t += tstep;
			}
		}	
	}	
	return 0;
}
/*
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
*/
static int TextureTrianglef(PlatformBitBuffer *bitBuff,
					float x1, float y1, float u1, float v1, float w1,
					float x2, float y2, float u2, float v2, float w2,
					float x3, float y3, float u3, float v3, float w3,
					SimpleImage* img, float* depth_buffer, Vec4 color)
{
	int end_x = bitBuff->width;
	int end_y = bitBuff->height-1;
	return TextureTriangle(bitBuff,
			ConvertRelToPlain(x1, 0, end_x), ConvertRelToPlain(y1, 0, end_y), u1, v1, w1,
			ConvertRelToPlain(x2, 0, end_x), ConvertRelToPlain(y2, 0, end_y), u2, v2, w2,
			ConvertRelToPlain(x3, 0, end_x), ConvertRelToPlain(y3, 0, end_y), u3, v3, w3,
			img, depth_buffer, color);
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

static void DrawImage(PlatformBitBuffer *bitBuff, SimpleImage *image, int min_x, int min_y)
{
	for (int j = 0; j < image->height; j++)
	{
		for (int i = 0; i < image->width; i++)
		{
			PlatformDrawPixel(bitBuff, min_x+i, min_y+j, image->pixels[image->width*(image->height-j-1)+i]);
		}
	}
}

static void DrawImageExceptColor(PlatformBitBuffer *bitBuff, SimpleImage *image, int min_x, int min_y, int vanish_color)
{
	for (int j = 0; j < image->height; j++)
	{
		for (int i = 0; i < image->width; i++)
		{
			if ((image->pixels[image->width*(image->height-j-1)+i] & vanish_color) != vanish_color)
				PlatformDrawPixel(bitBuff, min_x+i, min_y+j, image->pixels[image->width*(image->height-j-1)+i]);
		}
	}
}

static void DrawImageOnlyColor(PlatformBitBuffer *bitBuff, SimpleImage *image, int min_x, int min_y, int single_color)
{
	for (int j = 0; j < image->height; j++)
	{
		for (int i = 0; i < image->width; i++)
		{
			if (image->pixels[image->width*j+i] == single_color)
				PlatformDrawPixel(bitBuff, min_x+i, min_y+j, single_color);
		}
	}
}

static int CharToBMPFontCharIndex(char c)
{
	if (c >= 'A' && c <= 'Z')
	{
		return c - 'A';
	}
	if (c >= 'a' && c <= 'z')
	{
		return c - 'a';
	}
	if (c >= '0' && c <= '9')
	{
		return c - '0' + 26;
	}
	if (c == ',' || c == '.')
	{
		return 37;
	}
	if (c == ':')
	{
		return 38;
	}
	if (c == '-')
	{
		return 39;
	}
	if (c == ' ')
	{
		return 36;
	}
	return -1;
}

static void DrawBMPFontChar(PlatformBitBuffer *bitBuff, SimpleImage *font, int min_x, int min_y, char c)
{
	int font_color = MakeColor(0, 0, 0, 0);
	int index = CharToBMPFontCharIndex(c);
	if (index < 0)
	{
		return;
	}
	for (int j = 0; j < font->height; j++)
	{
		for (int i = 0; i < 16; i++)
		{
			if (font->pixels[font->width*j+i+index*16] == font_color)
				PlatformDrawPixel(bitBuff, min_x+i, min_y+j, font_color);
		}
	}
}

static void DrawBMPFontString(PlatformBitBuffer *bitBuff, SimpleImage *font, int min_x, int min_y, char *str, int str_count)
{
	for (int i = 0; i < str_count; i++)
	{
		DrawBMPFontChar(bitBuff, font, min_x + i*16, min_y, str[i]);
	}
}

/**********************************************************************/
// Command buffer stuff
/**********************************************************************/
typedef struct
{
	u32 command_type;
} CommandHeader;

typedef struct
{
	Vec4 color;
} Command_Clear;

typedef struct
{
	Vec4 color;
	Vec3 points[3];
	Vec3 tex_points[3];
	SimpleImage *tex;
} Command_Triangle;

typedef struct
{
	SimpleImage *img;
	int index;
} Command_InitTexture;

typedef struct
{
	uptr *vertices;
	i32 *strides;
} Command_SetVertices;

void ClearCommandBuffer(CommandBuffer *cmdBuff)
{
	cmdBuff->used = 0;
	cmdBuff->cmd_count = 0;
}

// returns a pointer to a command struct
u8* PushCommand(CommandBuffer *cmdBuff, u32 command_type)
{
	CommandHeader *header = (CommandHeader*)(cmdBuff->base_memory + cmdBuff->used);
	header->command_type = command_type;
	cmdBuff->used += sizeof(*header);
	
	u8 *cmd = cmdBuff->base_memory + cmdBuff->used;
	
	switch (command_type)
	{
		case COMMAND_TYPE_CLEAR:
		{
			cmdBuff->used += sizeof(Command_Clear);
		} break;
		
		case COMMAND_TYPE_SET_VERTICES:
		{
			cmdBuff->used += sizeof(Command_SetVertices);
		} break;
		
		case COMMAND_TYPE_TRIANGLE:
		{
			cmdBuff->used += sizeof(Command_Triangle);
		} break;
		/*
		case COMMAND_TYPE_INIT_TEXTURE:
		{
			cmdBuff->used += sizeof(Command_InitTexture);
		} break;*/
		
		default: ASSERT(0); break;
	}
	
	ASSERT(cmdBuff->used <= cmdBuff->max_size);
	cmdBuff->cmd_count++;
	return cmd;
}

void ClearCommand(CommandBuffer *cmdBuff, Vec4 color)
{
	Command_Clear *cmd = (Command_Clear*)PushCommand(cmdBuff, COMMAND_TYPE_CLEAR);
	cmd->color = color;
}

void SetVerticesCommand(CommandBuffer *cmdBuff, uptr *verts, i32 *strides)
{
	Command_SetVertices *cmd = (Command_SetVertices*)PushCommand(cmdBuff, COMMAND_TYPE_SET_VERTICES);
	cmd->vertices = verts;
	cmd->strides = strides;
}

// TODO: Vec3* tex_pts or Vec2* tex_pts?
void TriangleCommand(CommandBuffer *cmdBuff, Vec4 color, Vec3* pts, Tri tri, Vec3* tex_pts, Tri tex_tri, SimpleImage *img)
{
	Command_Triangle *cmd = (Command_Triangle*)PushCommand(cmdBuff, COMMAND_TYPE_TRIANGLE);
	cmd->color = color;
	cmd->points[0] = pts[tri.p1];
	cmd->points[1] = pts[tri.p2];
	cmd->points[2] = pts[tri.p3];
	cmd->tex_points[0] = tex_pts[tex_tri.p1];
	cmd->tex_points[1] = tex_pts[tex_tri.p2];
	cmd->tex_points[2] = tex_pts[tex_tri.p3];
	cmd->tex = img;
}
