/*
simple_3d_gfx.h - self explanatory header name
2023/09/25, peshqa
*/
#pragma once

#include <math.h>

typedef struct
{
	float x;
	float y;
	float z;
} Vec3f;

typedef struct
{
	Vec3f p[3];
} Tri3f;

void Vec3fSub(Vec3f &v1, Vec3f &v2, Vec3f &out)
{
	out.x = v1.x - v2.x;
	out.y = v1.y - v2.y;
	out.z = v1.z - v2.z;
}
void VecRaw3fSub(float *v1, float *v2, float *out)
{
	out[0] = v1[0] - v2[0];
	out[1] = v1[1] - v2[1];
	out[2] = v1[2] - v2[2];
}
void Vec3fAdd(Vec3f &v1, Vec3f &v2, Vec3f &out)
{
	out.x = v1.x + v2.x;
	out.y = v1.y + v2.y;
	out.z = v1.z + v2.z;
}
void VecRaw3fAdd(float *v1, float *v2, float *out)
{
	out[0] = v1[0] + v2[0];
	out[1] = v1[1] + v2[1];
	out[2] = v1[2] + v2[2];
}
void Vec3fMul(Vec3f &v1, Vec3f &v2, Vec3f &out)
{
	out.x = v1.x * v2.x;
	out.y = v1.y * v2.y;
	out.z = v1.z * v2.z;
}
void VecRaw3fMul(float *v1, float *v2, float *out)
{
	out[0] = v1[0] * v2[0];
	out[1] = v1[1] * v2[1];
	out[2] = v1[2] * v2[2];
}

int Vec3fDiv(Vec3f &v1, Vec3f &v2, Vec3f &out)
{
	// TODO: add divide by zero check?
	out.x = v1.x / v2.x;
	out.y = v1.y / v2.y;
	out.z = v1.z / v2.z;
	return 0;
}
int Vec3fNormalize(Vec3f &v)
{
	float len = sqrtf(v.x*v.x+v.y*v.y+v.z*v.z);
	v.x /= len; v.y /= len; v.z /= len;
	return 0;
}
int VecRaw3fNormalize(float *v, float *output)
{
	float len = sqrtf(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
	output[0] = v[0] / len; output[1] = v[1] / len; output[2] = v[2] / len;
	return 0;
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
void CrossProductVec3f(Vec3f &v1, Vec3f &v2, Vec3f &out)
{
	out.x = v1.y*v2.z - v1.z*v2.y;
	out.y = v1.z*v2.x - v1.x*v2.z;
	out.z = v1.x*v2.y - v1.y*v2.x;
}
float DotProductVec3f(Vec3f &v1, Vec3f &v2)
{
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}
float DotProductVecRaw3f(float *v1, float *v2)
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
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

void Vec3fMulByF(Vec3f &v1, float f, Vec3f &out)
{
	out.x = v1.x * f;
	out.y = v1.y * f;
	out.z = v1.z * f;
}
void VecRaw3fMulByFloat(float *v1, float f, float *out)
{
	out[0] = v1[0] * f;
	out[1] = v1[1] * f;
	out[2] = v1[2] * f;
}
void InitPointAtMat4x4(float *m, Vec3f &pos, Vec3f &target, Vec3f &up)
{
	for (int i = 0; i < 16; i++)
	{
		m[i] = 0;
	}
	Vec3f new_forward;
	Vec3f a;
	Vec3f new_up;
	Vec3f new_right;
	
	Vec3fSub(target, pos, new_forward);
	Vec3fNormalize(new_forward);
	
	Vec3fMulByF(new_forward, DotProductVec3f(up, new_forward), a);
	Vec3fSub(up, a, new_up);
	Vec3fNormalize(new_up);
	
	CrossProductVec3f(new_up, new_forward, new_right);
	
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

void PlaneVectorIntersection(float *plane_normal, float *plane_point, float *line_start, float *line_end, float* output)
{
	float normalized_plane_normal[3];
	VecRaw3fNormalize(plane_normal, normalized_plane_normal);
	float plane_d = -DotProductVecRaw3f(normalized_plane_normal, plane_point);
	float ad = DotProductVecRaw3f(line_start, normalized_plane_normal);
	float bd = DotProductVecRaw3f(line_end, normalized_plane_normal);
	float t = (-plane_d - ad) / (bd - ad);
	float start_to_end[3];
	float line_to_intersect[3];
	VecRaw3fSub(line_end, line_start, start_to_end);
	VecRaw3fMulByFloat(start_to_end, t, line_to_intersect);
	VecRaw3fAdd(line_start, line_to_intersect, output);
}

int ClipAgainstPlane(float *plane_normal, float *plane_point, float *points, int *in_tri, int *out_tri1, int *out_tri2)
{
	
	return 0;
}
