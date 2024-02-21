/*
simple_math.h - self explanatory header name
2023/09/25, peshqa
*/
#pragma once

#include <math.h>
#include <vector>

#define PI32 3.14159265359f

typedef union
{
	struct
	{
		float x;
		float y;
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
		float r;
		float g;
		float b;
	};
	float elem[3];
} Vec3;

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
	Vec3 res;
	res.x = v.x *f;
	res.y = v.y *f;
	res.z = v.z *f;
	return res;
}

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

Vec3 Vec3CrossProd(Vec3 v1, Vec3 v2)
{
	Vec3 res;
	res.x = v1.y*v2.z - v1.z*v2.y;
	res.y = v1.z*v2.x - v1.x*v2.z;
	res.z = v1.x*v2.y - v1.y*v2.x;
	return res;
}
float Vec3DotProd(Vec3 v1, Vec3 v2)
{
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}
Vec3 Vec3Normalize(Vec3 v)
{
	float len = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
	Vec3 res = v * (1.0f / len);
	return res;
}


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

void InitXRotMat4x4(float *m, float angle_rad/*roll*/)
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

void InitYRotMat4x4(float *m, float angle_rad/*pitch*/)
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

void InitZRotMat4x4(float *m, float angle_rad/*yaw*/)
{
	for (int i = 0; i < 16; i++)
	{
		m[i] = 0;
	}
	m[0] =  cosf(angle_rad);
	m[1] = -sinf(angle_rad);
	m[4] =  sinf(angle_rad);
	m[5] = cosf(angle_rad);
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
	//Vec3fSub(target, pos, new_forward);
	Vec3Normalize(new_forward);
	
	a = new_forward * Vec3DotProd(up, new_forward);
	//Vec3fMulByF(new_forward, DotProductVec3f(up, new_forward), a);
	new_up = up - a;
	//Vec3fSub(up, a, new_up);
	Vec3Normalize(new_up);
	
	//CrossProductVec3f((Vec3f&)new_up, (Vec3f&)new_forward, (Vec3f&)new_right);
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

// NOTE: only works for rotation or translation matrices
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

