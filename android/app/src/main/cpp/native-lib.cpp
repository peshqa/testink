#include <jni.h>
#include <string>

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