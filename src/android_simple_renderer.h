/*
android_simple_renderer.h - (Android specific) core of all smaller projects that draws things on the screen
2023/09/20, peshqa
*/
#pragma once

#include "platform_simple_renderer.h"

#include <stdlib.h>
#include <string>
#include <vector>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <jni.h>
#include <sstream>

typedef struct
{
	jobject asset_manager;
	AAsset* asset;
	JNIEnv* env;
	std::istringstream buffer;
} AndroidExtra;

int PlatformDrawPixel(PlatformBitBuffer *bitBuff, int x, int y, int color)
{
	if (x < 0 || x >= bitBuff->width || y < 0 || y >= bitBuff->height)
	{
		return -1;
	}
	((int*)(bitBuff->bits))[y*bitBuff->width+x] = color;
	return 0;
}

int ResizeAndroidBitBuffer(SharedState *state, int screenWidth, int screenHeight)
{
	/*PlatformBitBuffer* p = state->bitBuff;
	p->width = screenWidth;
	p->height = screenHeight;
	
	state->client_width = screenWidth;
	state->client_height = screenHeight;
	
	if (p->bits != 0)
	{
		delete [] (int*)p->bits;
	}
	p->bits = new int[screenWidth * screenHeight]{};*/
	
	return 0;
}

int MakeColor(int a, int r, int g, int b)
{
	return (a<<24) + (b<<16) + (g<<8) + r;
}

int InitSharedState(SharedState* s)
{
	// TODO: move initialization from main to here
	
	return 0;
}

int InitAssetManager(SharedState *s)
{
	AndroidExtra *extra = (AndroidExtra*)(s->extra);
	//extra->asset_manager = extra->env->NewGlobalRef(extra->asset_manager);
	return 0;
}
int OpenAssetFileA(SharedState *s, std::string &filename)
{
	AndroidExtra *extra = (AndroidExtra*)(s->extra);
	AAssetManager* mgr = AAssetManager_fromJava(extra->env, extra->asset_manager);
	extra->asset = AAssetManager_open(mgr, filename.c_str(), AASSET_MODE_BUFFER);
	//extra->file.open(filename);
	if (extra->asset != 0) {
        // Read the entire content into a buffer for line-by-line reading
        const void* data = AAsset_getBuffer(extra->asset);
        off_t size = AAsset_getLength(extra->asset);
        extra->buffer.str(std::string(static_cast<const char*>(data), size));
        return 0; // Success
    } else {
        return 1; // Failed to open asset
    }
	//return extra->file.fail();
}
int ReadAssetLineA(SharedState *s, std::string &line)
{
	AndroidExtra *extra = (AndroidExtra*)(s->extra);
	/*if (std::getline(extra->buffer, line)) {
        return 0; // Successfully read a line
    } else {
        return 1; // End of file or error
    }*/
	return !getline(extra->buffer, line).eof();
}
int CloseAssetFile(SharedState *s)
{
	AndroidExtra *extra = (AndroidExtra*)(s->extra);
	//extra->file.close();
	if (extra->asset != nullptr) {
        AAsset_close(extra->asset);
        extra->asset = nullptr;
    }
	return 0;
}
int TerminateAssetManager(SharedState *s)
{
	AndroidExtra *extra = (AndroidExtra*)(s->extra);
	extra->env->DeleteGlobalRef(extra->asset_manager);
	return 0;
}
