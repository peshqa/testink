#include "../../src/project.h"

#include <jni.h>

#include <android/log.h>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>
#include <string.h>
#include <android/window.h>

#define LOG(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "!NATIVE DEBUG!", __VA_ARGS__))

typedef struct
{
	AAssetManager *asset_manager;
	AAsset* asset;
	JNIEnv* env;
} AndroidExtra;

// returns 0 if failed (no memory commited)
u32 PlatformReadWholeFile(SharedState *s, char *filename, void *&p)
{
	AndroidExtra *extra = (AndroidExtra*)(s->extra);
	//AAssetManager* mgr = AAssetManager_fromJava(extra->env, extra->asset_manager);
	extra->asset = AAssetManager_open(extra->asset_manager, filename, AASSET_MODE_BUFFER);
	
	if (extra->asset != 0)
	{
		p = (void*)AAsset_getBuffer(extra->asset);
        off_t size = AAsset_getLength(extra->asset);
		return size;
	}
	// TODO: sould we close the asset?
	return 0;
}

int PlatformFreeFileMemory(SharedState* s, void*)
{
	return 0;
}
int MakeColor(int a, int r, int g, int b)
{
	return (a<<24) + (b<<16) + (g<<8) + r;
}

struct engine
{
    struct android_app* app;

    int active;
	
	SharedState shared_state;
};

static int32_t engine_handle_input(struct android_app* app, AInputEvent* event)
{
	
	engine* engine = (struct engine*)app->userData;
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
	{
		engine->shared_state.mouse_x = AMotionEvent_getX(event, 0);
		engine->shared_state.mouse_y = AMotionEvent_getY(event, 0);
		if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN) {
			engine->shared_state.is_lmb_down = 1;
		} else if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_UP) {
			engine->shared_state.is_lmb_down = 0;
		}
		return 1;
	}
    return 0;
}

static void engine_handle_cmd(android_app* app, int32_t cmd)
{
    engine* engine = (struct engine*)app->userData;
    switch (cmd)
    {
        case APP_CMD_INIT_WINDOW:
            if (engine->app->window != NULL)
            {
				engine->active = 1;
            }
            break;

        case APP_CMD_TERM_WINDOW:
            engine->active = 0;
            break;
        
        case APP_CMD_GAINED_FOCUS:
            engine->active = 1;
            break;

        case APP_CMD_LOST_FOCUS:
            engine->active = 0;
            break;
    }
}
extern "C" void android_main(android_app* state)
{
	engine engine;
    memset(&engine, 0, sizeof(engine));
	
	ANativeActivity_setWindowFlags(state->activity, AWINDOW_FLAG_KEEP_SCREEN_ON, 0);
	
	srand(time(NULL));
	int default_project = 0;
	int current_project = default_project;
	Project* projects{};
	InitProjectArray(projects);
	
	InitProjectFunction InitProjectFunc = (InitProjectFunction)(projects[current_project].InitFunc);
	UpdateProjectFunction UpdateProjectFunc = (UpdateProjectFunction)(projects[current_project].UpdateFunc);
	
	SharedState *shared_state = &engine.shared_state;
	shared_state->callback_update_func = projects[current_project].UpdateFunc;
	shared_state->is_running = 1;
	shared_state->asset_path = (char*)"";
	shared_state->is_accelerometer_active = 0; // for now these are inactive
	shared_state->is_gyroscope_active = 0; // in case the device doesn't support them
	shared_state->extra = new AndroidExtra{};
	AndroidExtra *extra = (AndroidExtra*)(shared_state->extra);
	extra->asset_manager = state->activity->assetManager;
	shared_state->project_memory = new u8[MEGABYTES(25)]{};
	if (InitProjectFunc(shared_state) != 0)
	{
		return;
	}
	shared_state->scale = 1;
	shared_state->bitBuff = new PlatformBitBuffer{};
	
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    while (1)
    {
        int ident;
        int events;
		android_poll_source* source;

        while ((ident=ALooper_pollAll(engine.active ? 0 : -1, NULL, &events, (void**)&source)) >= 0)
        {
            if (source != NULL)
            {
                source->process(state, source);
            }

            if (state->destroyRequested != 0)
            {
                return;
            }
        }
		
		ANativeWindow_Buffer native_screen;
        if (engine.active && state->window && !ANativeWindow_lock(state->window, &native_screen, 0))
        {
			PlatformBitBuffer* p = shared_state->bitBuff;
			p->bits = native_screen.bits;
			p->width = native_screen.width;
			p->height = native_screen.height;
			p->stride = native_screen.stride;
			p->is_top_to_bottom = 1;
			
			shared_state->client_width = native_screen.width;
			shared_state->client_height = native_screen.height;
			shared_state->delta_time = 0.013;
			UpdateProjectFunction UpdateFunc = (UpdateProjectFunction)(shared_state->callback_update_func);
			UpdateFunc(shared_state);
			ANativeWindow_unlockAndPost(state->window);
        }
    }
}