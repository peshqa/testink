/*
simple_3d_gfx.h - self explanatory header name
2023/09/25, peshqa
*/
#pragma once

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

void InitProjectionMat4x4(float *m, float fov, int is_fov_vertical, int screen_width, int screen_height, float z_near, float z_far)
{
	double pi = 3.1415926535;
	float f = 1/tanf(fov*pi/180/2);
	float q = z_far*(z_far-z_near);
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

void Vec3fSub(Vec3f *v1, Vec3f *v2, Vec3f *out)
{
	out->x = v1->x - v2->x;
	out->y = v1->y - v2->y;
	out->z = v1->z - v2->z;
}
void Vec3fAdd(Vec3f *v1, Vec3f *v2, Vec3f *out)
{
	out->x = v1->x + v2->x;
	out->y = v1->y + v2->y;
	out->z = v1->z + v2->z;
}
void Vec3fMul(Vec3f *v1, Vec3f *v2, Vec3f *out)
{
	out->x = v1->x * v2->x;
	out->y = v1->y * v2->y;
	out->z = v1->z * v2->z;
}
int Vec3fDiv(Vec3f *v1, Vec3f *v2, Vec3f *out)
{
	// TODO: add divide by zero check?
	out->x = v1->x / v2->x;
	out->y = v1->y / v2->y;
	out->z = v1->z / v2->z;
	return 0;
}
int Vec3fNormalize(Vec3f &v)
{
	float len = sqrtf(v.x*v.x+v.y*v.y+v.z*v.z);
	v.x /= len; v.y /= len; v.z /= len;
	return 0;
}