#pragma once
#include "project.h"
#include "android_simple_renderer.h"

#include <stdlib.h>
#include <chrono>
#include <string>
#include <vector>

extern "C" JNIEXPORT jlong JNICALL
Java_com_peshqa_testink_GameEngine_initData(
		JNIEnv *env,
		jobject thiz,
		jobject asset_mgr)
{
	srand(time(NULL));
	
	int default_project = 0;
	int current_project = default_project;
	Project* projects{};
	InitProjectArray(projects);
	
	InitProjectFunction InitProjectFunc = (InitProjectFunction)(projects[current_project].InitFunc);
	UpdateProjectFunction UpdateProjectFunc = (UpdateProjectFunction)(projects[current_project].UpdateFunc);
	
	SharedState *shared_state = new SharedState{};
	shared_state->callback_update_func = projects[current_project].UpdateFunc;
	shared_state->dir = 'l';
	
	shared_state->is_running = 1;
	
	shared_state->asset_path = "";
	
	shared_state->is_accelerometer_active = 0; // for now these are inactive
	shared_state->is_gyroscope_active = 0; // in case the device doesn't support them
	
	shared_state->extra = new AndroidExtra;
	AndroidExtra *extra = (AndroidExtra*)(shared_state->extra);
	extra->env = env;
	extra->asset_manager = extra->env->NewGlobalRef(asset_mgr);
	
	if (InitProjectFunc(shared_state) != 0)
	{
		return 0;
	}
	shared_state->scale = 1;
	shared_state->bitBuff = new PlatformBitBuffer{};
	return (jlong)shared_state;
}

extern "C" JNIEXPORT void JNICALL
Java_com_peshqa_testink_GameEngine_itHappen(
		JNIEnv *env,
		jobject thiz,
		jlong data,
		jchar dir)
{
	SharedState* state = (SharedState*)data;
	state->dir = (char)dir;
}

extern "C" JNIEXPORT void JNICALL
Java_com_peshqa_testink_GameEngine_updateRotationVector(
		JNIEnv *env,
		jobject thiz,
		jlong data,
		jfloat x,
		jfloat y,
		jfloat z,
		jfloat w)
{
	SharedState* state = (SharedState*)data;
	state->rot_vec_values[0] = (float)x;
	state->rot_vec_values[1] = (float)y;
	state->rot_vec_values[2] = (float)z;
	state->rot_vec_values[3] = (float)w;
}

extern "C" JNIEXPORT void JNICALL
Java_com_peshqa_testink_GameEngine_updateMouse(
		JNIEnv *env,
		jobject thiz,
		jlong data,
		jint x,
		jint y,
		jint is_down)
{
	SharedState* state = (SharedState*)data;
	state->mouse_x = (int)x;
	state->mouse_y = (int)y;
	state->is_lmb_down = (int)is_down;
}

extern "C" JNIEXPORT void JNICALL
Java_com_peshqa_testink_GameEngine_updateAndRenderGame(
		JNIEnv *env,
		jobject thiz,
		jlong data,
		jobject bitmap)
{
	AndroidBitmapInfo info;
	void *pixels;
	
	// Get info about the bitmap
	if (AndroidBitmap_getInfo(env, bitmap, &info) < 0)
	{
		return;
	}
	
	if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
	{
		return;
	}
	
	// Lock the bitmap to access its pixels
	if (AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0)
	{
		return;
	}
	
	SharedState* shared_state = (SharedState*)data;
	AndroidExtra *extra = (AndroidExtra*)(shared_state->extra);//
	extra->env = env;//
	shared_state->bitBuff->bits = pixels;
	PlatformBitBuffer* p = shared_state->bitBuff;
	p->width = info.width;
	p->height = info.height;
	
	shared_state->client_width = info.width;
	shared_state->client_height = info.height;
	UpdateProjectFunction UpdateFunc = (UpdateProjectFunction)(shared_state->callback_update_func);
	UpdateFunc(shared_state);
	
	AndroidBitmap_unlockPixels(env, bitmap);
	return;
}

extern "C" JNIEXPORT void JNICALL
Java_com_peshqa_testink_GameEngine_onScreenResize(
		JNIEnv *env,
		jobject thiz,
		jlong data,
		jint format,
		jint height,
		jint width)
{
	ResizeAndroidBitBuffer((SharedState*)data, (int)height, (int)width);
	return;
}