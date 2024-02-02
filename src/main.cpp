/*
main.cpp - entry point
Console isn't used by the app (initially), so 'wWinMain' is the main function.
2023/08/10, peshqa
*/
#define UNICODE

#include "project.h"
#include "simple_wasapi_renderer.h"
#include "simple_win32_renderer.h"
#include "wav_audio_loader.h"

#include <dsound.h>
#include <math.h>

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
typedef DIRECT_SOUND_CREATE(direct_sound_create);

static int InitDirectSound(HWND hwnd, int samples_per_size, int buffer_size, LPDIRECTSOUNDBUFFER  &secondary_buffer)
{
	HMODULE Library = LoadLibraryA("dsound.dll");
	
	if (!Library)
	{
		// TODO: logging?
		return -1;
	}
	
	direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(Library, "DirectSoundCreate");
	LPDIRECTSOUND direct_sound;
	
	if(!SUCCEEDED(DirectSoundCreate(0, &direct_sound, 0)))
	{
		// TODO: logging?
		return -2;
	}
	
	if(!SUCCEEDED(direct_sound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY)))
	{
		// TODO: logging?
		return -3;
	}
	
	//OutputDebugStringA("Set coop level\n");
	WAVEFORMATEX wave_format  = {};
    wave_format.wFormatTag = WAVE_FORMAT_PCM;
    wave_format.nChannels = 2;
    wave_format.nSamplesPerSec = samples_per_size;
    wave_format.wBitsPerSample = 16;
    wave_format.nBlockAlign = wave_format.nChannels * wave_format.wBitsPerSample / 8;
    wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
	
	DSBUFFERDESC buffer_desc = {};
	buffer_desc.dwSize = sizeof(buffer_desc);
	buffer_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	LPDIRECTSOUNDBUFFER  primary_buffer;
	
	if(!SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_desc, &primary_buffer, 0)))
	{
		// TODO: logging?
		return -4;
	}
	if(!SUCCEEDED(primary_buffer->SetFormat(&wave_format)))
	{
		// TODO: logging?
		return -5;
	}
	
	DSBUFFERDESC sec_buffer_desc = {};
	sec_buffer_desc.dwSize = sizeof(sec_buffer_desc);
	sec_buffer_desc.dwFlags = 0;
	sec_buffer_desc.dwBufferBytes = buffer_size;
    sec_buffer_desc.lpwfxFormat = &wave_format;
	//LPDIRECTSOUNDBUFFER  secondary_buffer;
	
	if(!SUCCEEDED(direct_sound->CreateSoundBuffer(&sec_buffer_desc, &secondary_buffer, 0)))
	{
		// TODO: logging?
		return -6;
	}
	//OutputDebugStringA("Direct sound initiated\n");
	
	return 0;
}

static float W32CalculateDeltaTime(uint64_t s, uint64_t e, LONGLONG pcfreq)
{
	float res = (float)(e - s) / pcfreq;
	return res;
}

static uint64_t W32GetPerfCounter()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return li.QuadPart;
}

typedef struct
{
	int samples_per_second;
	int bytes_per_sample;
	int sound_buffer_size;
	float t_sin;
	int latency_sample_count;
	int tone_hz;
	int tone_amplitude;
	int wave_period;
	//int half_wave_period;
	uint32_t running_sample_index;
	
	float *samples;
	int num_samples;
} SoundWaveInfo;

static void ZeroSoundBuffer(LPDIRECTSOUNDBUFFER sound_buffer, SoundWaveInfo *sound, DWORD lock_offset, DWORD bytes_to_lock)
{
	void *region1;
	DWORD region1_size;
	void *region2;
	DWORD region2_size;

	if(SUCCEEDED(sound_buffer->Lock(
			lock_offset,
			bytes_to_lock,
			&region1, &region1_size,
			&region2, &region2_size,
			0
		)))
	{
		int16_t *sample_out = (int16_t*)region1;
		for(int i = 0; i < region1_size / sound->bytes_per_sample; i++) {
			int16_t value = 0;
			*sample_out++ = value;
			*sample_out++ = value;
			sound->running_sample_index++;
		}

		sample_out = (int16_t*)region2;
		for(int i = 0; i < region2_size / sound->bytes_per_sample; i++) {
			int16_t value = 0;
			*sample_out++ = value;
			*sample_out++ = value;
			sound->running_sample_index++;
		}

		sound_buffer->Unlock(region1, region1_size, region2, region2_size);
	}
}

static void FillDirectSoundBuffer(LPDIRECTSOUNDBUFFER sound_buffer, SoundWaveInfo *sound, DWORD lock_offset, DWORD bytes_to_lock, PlatformSoundBuffer &soundBuff)
{
	void *region1;
	DWORD region1_size;
	void *region2;
	DWORD region2_size;

	if (SUCCEEDED(sound_buffer->Lock(
			lock_offset,
			bytes_to_lock,
			&region1, &region1_size,
			&region2, &region2_size,
			0
		)))
	{
		int idx = 0;
		int16_t *sample_out = (int16_t*)region1;
		for(int i = 0; i < region1_size / sound->bytes_per_sample; i++) {
			int16_t value = ((int16_t*)(soundBuff.buffer))[idx++];
			*sample_out++ = value;
			value = ((int16_t*)(soundBuff.buffer))[idx++];
			*sample_out++ = value;
			sound->running_sample_index++;
		}

		sample_out = (int16_t*)region2;
		for(int i = 0; i < region2_size / sound->bytes_per_sample; i++) {
			int16_t value = ((int16_t*)(soundBuff.buffer))[idx++];
			*sample_out++ = value;
			value = ((int16_t*)(soundBuff.buffer))[idx++];
			*sample_out++ = value;
			sound->running_sample_index++;
		}

		sound_buffer->Unlock(region1, region1_size, region2, region2_size);
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	srand(time(NULL));
	
	MMRESULT tbp = timeBeginPeriod(1);
	ASSERT(tbp == TIMERR_NOERROR);
	
	LARGE_INTEGER counter_per_sec;
	QueryPerformanceFrequency(&counter_per_sec);
	
	SharedState shared_state{};
	InitSharedState(&shared_state);
	
	/*int res = InitWASAPIRenderer();
	if (res != 0)
	{
		return 1;
	}*/
	
	
	int default_project = 0;
	int current_project = default_project;
	Project* projects{};
	InitProjectArray(projects);
	
	InitProjectFunction InitProjectFunc = (InitProjectFunction)(projects[current_project].InitFunc);
	UpdateProjectFunction UpdateProjectFunc = (UpdateProjectFunction)(projects[current_project].UpdateFunc);
	
	if (InitProjectFunc(&shared_state) != 0)
	{
		return -1;
	}
	
	shared_state.callback_update_func = projects[current_project].UpdateFunc;
	
	DWORD window_styles = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	
	RECT window_rect = {0, 0, shared_state.client_width, shared_state.client_height};
	AdjustWindowRectEx(&window_rect, window_styles, 0, 0);
	
    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"Main Window Class";
    
    WNDCLASS wc = { };

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Learn to Program Windows",    // Window text
        window_styles,            		// Window style

        // position and size
        100, 100, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
        );

    if (!hwnd)
    {
        return -2;
    }

	((W32Extra*)(shared_state.extra))->main_window = hwnd;
	SetWindowLongPtrW(
		hwnd,
		GWLP_USERDATA,
		(LONG_PTR)&shared_state
	);

	PlatformGoBorderlessFullscreen(&shared_state);
	
	// Sound stuff
	LPDIRECTSOUNDBUFFER sound_buffer;
	SoundWaveInfo sound = {};
	sound.samples_per_second = 48000;
	sound.latency_sample_count = sound.samples_per_second / 8;
	sound.bytes_per_sample = 4;
	sound.sound_buffer_size = sound.samples_per_second * sound.bytes_per_sample;
	sound.tone_hz = 256*2;
	sound.tone_amplitude = 3000;
	sound.wave_period = sound.samples_per_second / sound.tone_hz;
	
	int res = LoadWavFile(sound.samples, sound.num_samples);
	ASSERT(res == 0);
	
	sound.running_sample_index = 0;
	InitDirectSound(hwnd, sound.samples_per_second, sound.sound_buffer_size, sound_buffer);
	ZeroSoundBuffer(sound_buffer, &sound, 0, sound.sound_buffer_size);
	sound_buffer->Play(0, 0, DSBPLAY_LOOPING);
	
	// init screen buffer
	//FillPlatformBitBuffer(shared_state.bitBuff, MakeColor(255, 255, 255, 255));
	
	// Frame timing
	uint64_t prev_counter = W32GetPerfCounter();
    uint64_t prev_cycle_counter = __rdtsc();
	
	shared_state.soundBuff.buffer = new int[48000];
	ASSERT(shared_state.soundBuff.buffer != 0);

    // Run the message loop.
    MSG msg{};
	while (shared_state.is_running == 1)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				shared_state.is_running = 0;
				break;
			}
			
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		DWORD play_cursor;
        DWORD write_cursor;
		DWORD lock_offset;
		DWORD bytes_to_lock;
		DWORD target_cursor;
		
		if(SUCCEEDED(sound_buffer->GetCurrentPosition(&play_cursor, &write_cursor)))
		{
			lock_offset = sound.running_sample_index * sound.bytes_per_sample % sound.sound_buffer_size;
			target_cursor = (play_cursor + sound.latency_sample_count * sound.bytes_per_sample) % sound.sound_buffer_size;
			if(lock_offset > target_cursor) {
				bytes_to_lock = sound.sound_buffer_size - lock_offset + target_cursor;
			} else {
				bytes_to_lock = target_cursor - lock_offset;
			}
			//FillSoundBuffer(sound_buffer, &sound, lock_offset, bytes_to_lock);
			shared_state.soundBuff.samples_to_fill = bytes_to_lock / sound.bytes_per_sample;
        } else {
			ASSERT(!"GetCurrentPosition failed!");
		}
		
		//shared_state.soundBuff.samples_to_fill = 0;
		UpdateProjectFunc(&shared_state);
		FillDirectSoundBuffer(sound_buffer, &sound, lock_offset, bytes_to_lock, shared_state.soundBuff);
		
		HDC hdc = GetDC(hwnd);
		((W32Extra*)(shared_state.extra))->hdc = hdc;
		PlatformUpdateDisplay(&shared_state, shared_state.client_width, shared_state.client_height);
		ReleaseDC(hwnd, hdc);
		
		
		// Timing
		u64 counter_before_sleep = W32GetPerfCounter();
		float elapsed_ms_before_sleep = 1000.0f * W32CalculateDeltaTime(prev_counter, counter_before_sleep, counter_per_sec.QuadPart);

		int ms_to_sleep;
		if (shared_state.max_fps > 0)
		{
			float sec_per_frame = 1.0f / shared_state.max_fps;
			if (elapsed_ms_before_sleep < sec_per_frame * 1000.0f)
			{
				ms_to_sleep = sec_per_frame*1000.0f - elapsed_ms_before_sleep;
				
				if (ms_to_sleep > 0)
				{
					Sleep(ms_to_sleep);
				}
			} else {
				OutputDebugStringA("too slow!\n");
			}
		}
		
		uint64_t current_cycle_counter = __rdtsc();
        uint64_t current_counter = W32GetPerfCounter();

        int64_t counter_elapsed = current_counter - prev_counter;
        uint64_t cycles_elapsed = current_cycle_counter - prev_cycle_counter;

        
        float fps = (float)counter_per_sec.QuadPart / counter_elapsed;
        float mega_cycles_per_frame = (float)cycles_elapsed / (1000.0f * 1000.0f);
		
		
		float elapsed_ms = 1000.0f * W32CalculateDeltaTime(prev_counter, W32GetPerfCounter(), counter_per_sec.QuadPart);
		shared_state.delta_time = elapsed_ms / 1000.0f;
		
		char Buffer[256];
		sprintf(Buffer, "ms b4 sleep: %.2f, ms/f: %.2f, sleep: %d,  fps: %.2f,  mc/f: %.2f\n", elapsed_ms_before_sleep, elapsed_ms, ms_to_sleep, fps, mega_cycles_per_frame);
		OutputDebugStringA(Buffer);
		
		prev_counter = current_counter;
        prev_cycle_counter = current_cycle_counter;
	}
	
	// TODO: on exit audio
	sound_buffer->Stop();

    return 0;
}
