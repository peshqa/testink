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
		jobject thiz)
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
	
	shared_state->callback_update_func = projects[current_project].UpdateFunc;
	shared_state->dir = 'l';
	
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