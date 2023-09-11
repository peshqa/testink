#include <jni.h>
#include <string>

#include <android/bitmap.h>

// src path: testink\android\app\src\main\cpp
// dest path: testink\src
#include "..\..\..\..\..\src\test_string.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_peshqa_testink_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(s.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_peshqa_testink_GameEngine_updateBitmap(
		JNIEnv *env,
		jobject thiz,
		jobject canvas)
{
	AndroidBitmapInfo info;
	void *pixels;
	
	// Get info about the Canvas bitmap
	if (AndroidBitmap_getInfo(env, canvas, &info) < 0)
	{
		return;
	}
	
	if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
	{
		return;
	}
	
	// Lock the Canvas bitmap to access its pixels
	if (AndroidBitmap_lockPixels(env, canvas, &pixels) < 0)
	{
		return;
	}
	
	// write pixels here
	for (int i = 0; i < info.height; i++)
	{
		for (int j = 0; j < info.width; j++)
		{
			// Format: RRGGBBAA
			((int32_t*)(pixels))[i*info.width+j] = 0xFF00FFFF+(i<<8)+(j<<16);
		}
	}
	
	// Unlock the Canvas to apply changes
	AndroidBitmap_unlockPixels(env, canvas);
}

