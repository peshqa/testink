/*
simple_math.h - self explanatory header name
2023/09/25, peshqa
*/
#pragma once

#include <math.h>

#ifdef USING_SIMD_SSE
#include <xmmintrin.h> // SIMD / Intel Intrinsics
#endif

#ifdef USING_SIMD_NEON
#include <arm_neon.h>
#endif

#define PI32 3.14159265359f

#define ABS(a) ((a) > 0 ? (a) : -(a))

typedef union
{
	struct
	{
		int p1;
		int p2;
		int p3;
	};
	int elem[3];
} Tri;

typedef union
{
	struct
	{
		float x;
		float y;
	};
	struct
	{
		float u;
		float v;
	};
	struct
	{
		float width;
		float height;
	};
	float elem[2];
} Vec2;

typedef union
{
	struct
	{
		float x;
		float y;
		float z;
	};
	struct
	{
		float u;
		float v;
		float w;
	};
	struct
	{
		float r;
		float g;
		float b;
	};
	struct
	{
		Vec2 xy;
		float _pad;
	};
	float elem[3];
} Vec3;

typedef union
{
	struct
	{
		float x;
		float y;
		float z;
		float w;
	};
	struct
	{
		float r;
		float g;
		float b;
		float a;
	};
	struct
	{
		Vec3 xyz;
		float _pad;
	};
	float elem[4];
	
#if 0
	__m128 simd;
#endif
} Vec4;

// NOTE: matrices use column major order
typedef union
{
	struct
	{
		Vec4 col[4];
	};
	float elem[4][4];
} Mat4;

Vec2 operator+(Vec2 a, Vec2 b)
{
	Vec2 res;
	res.x = a.x + b.x;
	res.y = a.y + b.y;
	return res;
}
Vec3 operator+(Vec3 a, Vec3 b)
{
	Vec3 res;
	res.x = a.x + b.x;
	res.y = a.y + b.y;
	res.z = a.z + b.z;
	return res;
}

Vec3 &operator+=(Vec3 &a, Vec3 b)
{
	a = a + b;
	return a;
}

Vec2 operator-(Vec2 a, Vec2 b)
{
	Vec2 res;
	res.x = a.x - b.x;
	res.y = a.y - b.y;
	return res;
}
Vec3 operator-(Vec3 a, Vec3 b)
{
	Vec3 res;
	res.x = a.x - b.x;
	res.y = a.y - b.y;
	res.z = a.z - b.z;
	return res;
}

Vec3 &operator-=(Vec3 &a, Vec3 b)
{
	a = a - b;
	return a;
}

Vec2 operator*(Vec2 v, float f)
{
	Vec2 res;
	res.x = v.x * f;
	res.y = v.y * f;
	return res;
}
Vec2 operator*(float f, Vec2 v)
{
	Vec2 res;
	res.x = v.x *f;
	res.y = v.y *f;
	return res;
}
Vec3 operator*(Vec3 v, float f)
{
	Vec3 res;
	res.x = v.x * f;
	res.y = v.y * f;
	res.z = v.z * f;
	return res;
}
Vec3 operator*(float f, Vec3 v)
{
	return v * f;
}

Vec3 operator/(Vec3 v, float f)
{
	Vec3 res;
	res.x = v.x / f;
	res.y = v.y / f;
	res.z = v.z / f;
	return res;
}

Vec3 MakeVec3(float x, float y, float z)
{
	Vec3 res;
	res.x = x;
	res.y = y;
	res.z = z;
	return res;
}
Vec4 MakeVec4(float x, float y, float z, float w)
{
	Vec4 res;
	res.x = x;
	res.y = y;
	res.z = z;
	res.w = w;
	return res;
}

Vec3 Vec3CrossProd(Vec3 v1, Vec3 v2)
{
	Vec3 res;
	res.x = v1.y*v2.z - v1.z*v2.y;
	res.y = v1.z*v2.x - v1.x*v2.z;
	res.z = v1.x*v2.y - v1.y*v2.x;
	return res;
}

Vec3 Vec3Normalize(Vec3 v)
{
	float len = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
	Vec3 res = v * (1.0f / len);
	return res;
}

float Vec3DotProd(Vec3 v1, Vec3 v2)
{
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

Vec4 operator*(Vec4 vec, Mat4 mat)
{
	Vec4 res = {};
#if 1
	for (int col = 0; col < 4; col++)
	{
		for (int row = 0; row < 4; row++)
		{
			res.elem[col] += mat.elem[col][row]*vec.elem[row];
		}
	}
#else
	res.simd = _mm_mul_ps(_mm_shuffle_ps(vec.simd, vec.simd, 0x00), mat.col[0].simd);
    res.simd = _mm_add_ps(res.simd, _mm_mul_ps(_mm_shuffle_ps(vec.simd, vec.simd, 0x55), mat.col[1].simd));
    res.simd = _mm_add_ps(res.simd, _mm_mul_ps(_mm_shuffle_ps(vec.simd, vec.simd, 0xaa), mat.col[2].simd));
    res.simd = _mm_add_ps(res.simd, _mm_mul_ps(_mm_shuffle_ps(vec.simd, vec.simd, 0xff), mat.col[3].simd));
#endif
	return res;
}
#if 0
Vec4 operator*(Mat4 mat, Vec4 vec)
{
	Vec4 res; // TODO: verify the calculations are correct?
	res.simd = _mm_mul_ps(_mm_shuffle_ps(vec.simd, vec.simd, 0x00), mat.col[0].simd);
    res.simd = _mm_add_ps(res.simd, _mm_mul_ps(_mm_shuffle_ps(vec.simd, vec.simd, 0x55), mat.col[1].simd));
    res.simd = _mm_add_ps(res.simd, _mm_mul_ps(_mm_shuffle_ps(vec.simd, vec.simd, 0xaa), mat.col[2].simd));
    res.simd = _mm_add_ps(res.simd, _mm_mul_ps(_mm_shuffle_ps(vec.simd, vec.simd, 0xff), mat.col[3].simd));
	return vec * mat;
}
#endif

Mat4 operator*(Mat4 left, Mat4 right)
{
	Mat4 res;
#if 1
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			res.elem[j][i] = 0;
			for (int k = 0; k < 4; k++)
			{
				res.elem[j][i] += left.elem[k][i]*right.elem[j][k];
			}
		}
	}
#else
	res.col[0] = left * right.col[0];
    res.col[1] = left * right.col[1];
    res.col[2] = left * right.col[2];
    res.col[3] = left * right.col[3];
#endif
	return res;
}

Mat4 MakeXRotMat4(float angle_rad)
{
	Mat4 res = {};
	res.elem[1][1] =  cosf(angle_rad);
	res.elem[2][1] = -sinf(angle_rad);
	res.elem[1][2] =  sinf(angle_rad);
	res.elem[2][2] =  cosf(angle_rad);

	res.elem[0][0] = 1.0f;
	res.elem[3][3] = 1.0f;
	return res;
}


Mat4 MakeYRotMat4(float angle_rad)
{
	Mat4 res = {};
	res.elem[0][0] =  cosf(angle_rad);
	res.elem[2][0] =  sinf(angle_rad);
	res.elem[0][2] = -sinf(angle_rad);
	res.elem[2][2] =  cosf(angle_rad);

	res.elem[1][1] = 1.0f;
	res.elem[3][3] = 1.0f;
	return res;
}

Mat4 MakeZRotMat4(float angle_rad)
{
	Mat4 res = {};
	res.elem[0][0] =  cosf(angle_rad);
	res.elem[1][0] = -sinf(angle_rad);
	res.elem[0][1] =  sinf(angle_rad);
	res.elem[1][1] =  cosf(angle_rad);

	res.elem[2][2] = 1.0f;
	res.elem[3][3] = 1.0f;
	return res;
}

Mat4 MakeTranslationMat4(Vec3 v)
{
	Mat4 res = {};
	res.elem[0][0] = 1.0f;
	res.elem[1][1] = 1.0f;
	res.elem[2][2] = 1.0f;
	res.elem[0][3] = v.x;
	res.elem[1][3] = v.y;
	res.elem[2][3] = v.z;
	res.elem[3][3] = 1.0f;
	return res;
}


Mat4 MakeScaleMat4(Vec3 v)
{
	Mat4 res = {};
	res.elem[0][0] = v.x;
	res.elem[1][1] = v.y;
	res.elem[2][2] = v.z;
	res.elem[3][3] = 1.0f;
	return res;
}


Mat4 MakePointAtMat4(Vec3 pos, Vec3 target, Vec3 up)
{
	Mat4 res = {};
	Vec3 new_forward;
	Vec3 a;
	Vec3 new_up;
	Vec3 new_right;
	
	new_forward = Vec3Normalize(target - pos);
	
	a = new_forward * Vec3DotProd(up, new_forward);
	new_up = Vec3Normalize(up - a);
	
	new_right = Vec3CrossProd(new_up, new_forward);
	
	res.elem[0][0] = new_right.x;
	res.elem[1][0] = new_right.y;
	res.elem[2][0] = new_right.z;
	res.elem[0][1] = new_up.x;
	res.elem[1][1] = new_up.y;
	res.elem[2][1] = new_up.z;
	res.elem[0][2] = new_forward.x;
	res.elem[1][2] = new_forward.y;
	res.elem[2][2] = new_forward.z;
	res.elem[0][3] = pos.x;
	res.elem[1][3] = pos.y;
	res.elem[2][3] = pos.z;
	res.elem[3][3] = 1;
	return res;
}

// NOTE: only works for rotation or translation matrices
Mat4 QuickInverseMat4(Mat4 m)
{
	Mat4 res = {};
	res.elem[0][0] = m.elem[0][0];
	res.elem[1][0] = m.elem[1][0];
	res.elem[2][0] = m.elem[2][0];
	res.elem[3][0] = 0.0f;
	res.elem[0][1] = m.elem[0][1];
	res.elem[1][1] = m.elem[1][1];
	res.elem[2][1] = m.elem[2][1];
	res.elem[3][1] = 0.0f;
	res.elem[0][2] = m.elem[0][2];
	res.elem[1][2] = m.elem[1][2];
	res.elem[2][2] = m.elem[2][2];
	res.elem[3][2] = 0.0f;
	res.elem[0][3] = -(m.elem[0][3]*res.elem[0][0] + m.elem[1][3]*res.elem[0][1] + m.elem[2][3]*res.elem[0][2]);
	res.elem[1][3] = -(m.elem[0][3]*res.elem[1][0] + m.elem[1][3]*res.elem[1][1] + m.elem[2][3]*res.elem[1][2]);
	res.elem[2][3] = -(m.elem[0][3]*res.elem[2][0] + m.elem[1][3]*res.elem[2][1] + m.elem[2][3]*res.elem[2][2]);
	res.elem[3][3] = 1.0f;
	return res;
}


Mat4 MakeProjectionMat4(float fov, int is_fov_vertical, int screen_width, int screen_height, float z_near, float z_far)
{
	Mat4 res = {};
	float f = 1/tanf(fov*PI32/180*0.5f);
	float q = z_far/(z_far-z_near);
	float aspect_ratio;

	if (is_fov_vertical != 0)
	{
		aspect_ratio = (float)screen_height / screen_width;
		res.elem[0][0] = aspect_ratio*f;
		res.elem[1][1] = f;
	} else {
		aspect_ratio = (float)screen_width / screen_height;
		res.elem[0][0] = f;
		res.elem[1][1] = aspect_ratio*f;
	}
	
	res.elem[2][2] = q;
	res.elem[3][2] = 1.0f;
	res.elem[2][3] = -q*z_near;
	return res;
}

void PlaneVectorIntersection(Vec3 plane_normal, Vec3 plane_point, Vec3 line_start, Vec3 line_end, Vec3* output, float &t)
{
	Vec3 normalized_plane_normal;
	normalized_plane_normal = Vec3Normalize(plane_normal);
	float plane_d = -Vec3DotProd(normalized_plane_normal, plane_point);
	float ad = Vec3DotProd(line_start, normalized_plane_normal);
	float bd = Vec3DotProd(line_end, normalized_plane_normal);
	t = (-plane_d - ad) / (bd - ad);
	Vec3 start_to_end;
	Vec3 line_to_intersect;
	//VecRaw3fSub(line_end, line_start, start_to_end);
	start_to_end = line_end - line_start;
	//VecRaw3fMulByFloat(start_to_end, t, line_to_intersect);
	line_to_intersect = start_to_end * t;
	//VecRaw3fAdd(line_start.elem, line_to_intersect.elem, output);
	*output = line_start + line_to_intersect;
}
float dist(Vec3 plane_normal, Vec3 plane_point, Vec3 v)
{
	return (plane_normal.x * v.x + plane_normal.y * v.y + plane_normal.z * v.z - Vec3DotProd(plane_normal, plane_point));
}
// Returns the number of newly formed triangles
int ClipAgainstPlane(Vec3 plane_normal, Vec3 plane_point, Vec3 *points, int *points_count, Vec3 *tex_points, int *tex_points_count,
						int *in_tri, int *in_tex,
						int *out_tri1, int *out_tex1,
						int *out_tri2, int *out_tex2)
{
	plane_normal = Vec3Normalize(plane_normal);

	

	// Create two temporary storage arrays to classify points either side of plane
	// If distance sign is positive, point lies on "inside" of plane
	int inside_points[3];  int nInsidePointCount = 0;
	int outside_points[3]; int nOutsidePointCount = 0;
	int inside_tex_points[3];  int nInsideTexPointCount = 0;
	int outside_tex_points[3]; int nOutsideTexPointCount = 0;

	// Get signed distance of each point in triangle to plane
	float d0 = dist(plane_normal, plane_point, points[in_tri[0]]);
	float d1 = dist(plane_normal, plane_point, points[in_tri[1]]);
	float d2 = dist(plane_normal, plane_point, points[in_tri[2]]);

	if (d0 >= 0)
	{
		inside_points[nInsidePointCount++] = in_tri[0]; inside_tex_points[nInsideTexPointCount++] = in_tex[0];
	} else {
		outside_points[nOutsidePointCount++] = in_tri[0]; outside_tex_points[nOutsideTexPointCount++] = in_tex[0];
	}
	if (d1 >= 0)
	{
		inside_points[nInsidePointCount++] = in_tri[1]; inside_tex_points[nInsideTexPointCount++] = in_tex[1];
	} else {
		outside_points[nOutsidePointCount++] = in_tri[1]; outside_tex_points[nOutsideTexPointCount++] = in_tex[1];
	}
	if (d2 >= 0)
	{
		inside_points[nInsidePointCount++] = in_tri[2]; inside_tex_points[nInsideTexPointCount++] = in_tex[2];
	} else {
		outside_points[nOutsidePointCount++] = in_tri[2]; outside_tex_points[nOutsideTexPointCount++] = in_tex[2];
	}
	
	float t;

	if (nInsidePointCount == 0)
	{
		return 0;
	}

	if (nInsidePointCount == 3)
	{
		// All points lie on the inside of plane, so do nothing
		// and allow the triangle to simply pass through
		out_tri1[0] = in_tri[0];
		out_tri1[1] = in_tri[1];
		out_tri1[2] = in_tri[2];
		
		out_tex1[0] = in_tex[0];
		out_tex1[1] = in_tex[1];
		out_tex1[2] = in_tex[2];

		return 1;
	}

	if (nInsidePointCount == 1 && nOutsidePointCount == 2)
	{
		// form a triangle
		out_tri1[0] = inside_points[0];
		out_tex1[0] = inside_tex_points[0];

		Vec3 np1;
		Vec3 np2;
		Vec3 ntp1;
		Vec3 ntp2;
		
		PlaneVectorIntersection(plane_normal, plane_point, points[inside_points[0]], points[outside_points[0]], &np1, t);
		ntp1.x = t * (tex_points[outside_tex_points[0]].x - tex_points[inside_tex_points[0]].x) + tex_points[inside_tex_points[0]].x;
		ntp1.y = t * (tex_points[outside_tex_points[0]].y - tex_points[inside_tex_points[0]].y) + tex_points[inside_tex_points[0]].y;
		ntp1.z = t * (tex_points[outside_tex_points[0]].z - tex_points[inside_tex_points[0]].z) + tex_points[inside_tex_points[0]].z;
		out_tex1[1] = *tex_points_count;
		tex_points[*tex_points_count] = (ntp1);
		*tex_points_count = *tex_points_count + 1;
		
		PlaneVectorIntersection(plane_normal, plane_point, points[inside_points[0]], points[outside_points[1]], &np2, t);
		ntp2.x = t * (tex_points[outside_tex_points[1]].x - tex_points[inside_tex_points[0]].x) + tex_points[inside_tex_points[0]].x;
		ntp2.y = t * (tex_points[outside_tex_points[1]].y - tex_points[inside_tex_points[0]].y) + tex_points[inside_tex_points[0]].y;
		ntp2.z = t * (tex_points[outside_tex_points[1]].z - tex_points[inside_tex_points[0]].z) + tex_points[inside_tex_points[0]].z;
		out_tex1[2] = *tex_points_count;
		tex_points[*tex_points_count] = (ntp2);
		*tex_points_count = *tex_points_count + 1;
		
		out_tri1[1] = *points_count;
		points[*points_count] = np1;
		*points_count = *points_count + 1;
		out_tri1[2] = *points_count;
		points[*points_count] = np2;
		*points_count = *points_count + 1;

		return 1;
	}

	if (nInsidePointCount == 2 && nOutsidePointCount == 1)
	{
		// form a quad
		Vec3 np1;
		Vec3 np2;
		Vec3 ntp1;
		Vec3 ntp2;

		out_tri1[0] = inside_points[0];
		out_tri1[1] = inside_points[1];
		out_tex1[0] = inside_tex_points[0];
		out_tex1[1] = inside_tex_points[1];
		
		PlaneVectorIntersection(plane_normal, plane_point, points[inside_points[0]], points[outside_points[0]], &np1, t);
		ntp1.x = t * (tex_points[outside_tex_points[0]].x - tex_points[inside_tex_points[0]].x) + tex_points[inside_tex_points[0]].x;
		ntp1.y = t * (tex_points[outside_tex_points[0]].y - tex_points[inside_tex_points[0]].y) + tex_points[inside_tex_points[0]].y;
		ntp1.z = t * (tex_points[outside_tex_points[0]].z - tex_points[inside_tex_points[0]].z) + tex_points[inside_tex_points[0]].z;
		out_tex1[2] = *tex_points_count;
		tex_points[*tex_points_count] = (ntp1);
		*tex_points_count = *tex_points_count + 1;
		
		out_tri1[2] = *points_count;
		points[*points_count] = np1;
		*points_count = *points_count + 1;

		out_tri2[0] = inside_points[1];
		out_tex2[0] = inside_tex_points[1];
		out_tri2[1] = out_tri1[2];
		out_tex2[1] = out_tex1[2];
		PlaneVectorIntersection(plane_normal, plane_point, points[inside_points[1]], points[outside_points[0]], &np2, t);
		ntp2.x = t * (tex_points[outside_tex_points[0]].x - tex_points[inside_tex_points[1]].x) + tex_points[inside_tex_points[1]].x;
		ntp2.y = t * (tex_points[outside_tex_points[0]].y - tex_points[inside_tex_points[1]].y) + tex_points[inside_tex_points[1]].y;
		ntp2.z = t * (tex_points[outside_tex_points[0]].z - tex_points[inside_tex_points[1]].z) + tex_points[inside_tex_points[1]].z;
		out_tex2[2] = *tex_points_count;
		tex_points[*tex_points_count] = (ntp2);
		*tex_points_count = *tex_points_count + 1;
		
		out_tri2[2] = *points_count;
		points[*points_count] = np2;
		*points_count = *points_count + 1;

		return 2;
	}
	return -1;
}

/* UNUSED CODE
// some row major matrix operations
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
void InitPointAtMat4x4(float *m, Vec3 &pos, Vec3 &target, Vec3 &up)
{
	for (int i = 0; i < 16; i++)
	{
		m[i] = 0;
	}
	Vec3 new_forward;
	Vec3 a;
	Vec3 new_up;
	Vec3 new_right;
	
	new_forward = target - pos;
	Vec3Normalize(new_forward);
	
	a = new_forward * Vec3DotProd(up, new_forward);
	new_up = up - a;
	Vec3Normalize(new_up);
	
	new_right = Vec3CrossProd(new_up, new_forward);
	
	m[0] = new_right.x;
	m[1] = new_right.y;
	m[2] = new_right.z;
	m[4] = new_up.x;
	m[5] = new_up.y;
	m[6] = new_up.z;
	m[8] = new_forward.x;
	m[9] = new_forward.y;
	m[10] = new_forward.z;
	m[12] = pos.x;
	m[13] = pos.y;
	m[14] = pos.z;
	m[15] = 1;
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
void InitXRotMat4x4(float *m, float angle_rad)
{
	for (int i = 0; i < 16; i++)
	{
		m[i] = 0;
	}
	m[0] = 1;
	m[5] =  cosf(angle_rad);
	m[6] = -sinf(angle_rad);
	m[9] =  sinf(angle_rad);
	m[10] = cosf(angle_rad);
	m[15] = 1;
}
void InitYRotMat4x4(float *m, float angle_rad)
{
	for (int i = 0; i < 16; i++)
	{
		m[i] = 0;
	}
	m[0] =  cosf(angle_rad);
	m[2] =  sinf(angle_rad);
	m[5] =  1;
	m[8] = -sinf(angle_rad);
	m[10] = cosf(angle_rad);
	m[15] = 1;
}
void InitZRotMat4x4(float *m, float angle_rad)
{
	for (int i = 0; i < 16; i++)
	{
		m[i] = 0;
	}
	m[0] =  cosf(angle_rad);
	m[1] = -sinf(angle_rad);
	m[4] =  sinf(angle_rad);
	m[5] =  cosf(angle_rad);
	m[10] = 1;
	m[15] = 1;
}
void InitTranslationMat4x4(float *m, float x, float y, float z)
{
	for (int i = 0; i < 16; i++)
	{
		m[i] = 0;
	}
	m[0] = 1;
	m[5] = 1;
	m[10] = 1;
	m[12] = x;
	m[13] = y;
	m[14] = z;
	m[15] = 1;
}
void Mat4x4QuickInverse(float *m, float *out)
{
	out[0] = m[0];
	out[1] = m[4];
	out[2] = m[8];
	out[3] = 0.0f;
	out[4] = m[1];
	out[5] = m[5];
	out[6] = m[9];
	out[7] = 0.0f;
	out[8] = m[2];
	out[9] = m[6];
	out[10] = m[10];
	out[11] = 0.0f;
	out[12] = -(m[12]*out[0] + m[13]*out[4] + m[14]*out[8]);
	out[13] = -(m[12]*out[1] + m[13]*out[5] + m[14]*out[9]);
	out[14] = -(m[12]*out[2] + m[13]*out[6] + m[14]*out[10]);
	out[15] = 1.0f;
}
void InitScaleMat4x4(float *m, float x, float y, float z)
{
	for (int i = 0; i < 16; i++)
	{
		m[i] = 0;
	}
	m[0] = x;
	m[5] = y;
	m[10] = z;
	m[15] = 1;
}
void InitProjectionMat4x4(float *m, float fov, int is_fov_vertical, int screen_width, int screen_height, float z_near, float z_far)
{
	double pi = 3.1415926535;
	float f = 1/tanf(fov*pi/180*0.5f);
	float q = z_far/(z_far-z_near);
	float aspect_ratio;
	
	for (int i = 0; i < 16; i++)
	{
		m[i] = 0;
	}
	
	if (is_fov_vertical != 0)
	{
		aspect_ratio = (float)screen_height/screen_width;
		m[0] = aspect_ratio*f;
		m[5] = f;
	} else {
		aspect_ratio = (float)screen_width/screen_height;
		m[0] = f;
		m[5] = aspect_ratio*f;
	}
	
	m[10] = q;
	m[11] = 1;
	m[14] = -q*z_near;
}
*/
