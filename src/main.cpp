/*
main.cpp - entry point
Console isn't used by the app (initially), so 'wWinMain' is the main function.
2023/08/10, peshqa
*/
#define UNICODE

#include "project.h"
#include "simple_wasapi_renderer.h"
#include "simple_win32_renderer.h"

#include <dsound.h>
#include <math.h>

#define PI32 3.14159265359

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
typedef DIRECT_SOUND_CREATE(direct_sound_create);

void InitDirectSound(HWND hwnd, int samples_per_size, int buffer_size, LPDIRECTSOUNDBUFFER  &secondary_buffer)
{
	HMODULE Library = LoadLibraryA("dsound.dll");
	
	if (!Library)
	{
		// TODO: logging?
		return;
	}
	
	direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(Library, "DirectSoundCreate");
	LPDIRECTSOUND direct_sound;
	
	if(!SUCCEEDED(DirectSoundCreate(0, &direct_sound, 0)))
	{
		// TODO: logging?
		return;
	}
	
	if(!SUCCEEDED(direct_sound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY)))
	{
		// TODO: logging?
		return;
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
		return;
	}
	if(!SUCCEEDED(primary_buffer->SetFormat(&wave_format)))
	{
		// TODO: logging?
		return;
	}
	//OutputDebugStringA("Set sec buffer format\n");
	
	DSBUFFERDESC sec_buffer_desc = {};
	sec_buffer_desc.dwSize = sizeof(sec_buffer_desc);
	sec_buffer_desc.dwFlags = 0;
	sec_buffer_desc.dwBufferBytes = buffer_size;
    sec_buffer_desc.lpwfxFormat = &wave_format;
	//LPDIRECTSOUNDBUFFER  secondary_buffer;
	
	if(!SUCCEEDED(direct_sound->CreateSoundBuffer(&sec_buffer_desc, &secondary_buffer, 0)))
	{
		// TODO: logging?
		return;
	}
	OutputDebugStringA("Direct sound initiated\n");
	
	
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	srand(time(NULL));
	
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
		return 0;
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

    if (hwnd == NULL)
    {
        return 0;
    }

	((W32Extra*)(shared_state.extra))->main_window = hwnd;
	SetWindowLongPtrW(
		hwnd,
		GWLP_USERDATA,
		(LONG_PTR)&shared_state
	);

	PlatformGoBorderlessFullscreen(&shared_state);
	
	// Sound stuff
	LPDIRECTSOUNDBUFFER  sound_buffer;
	int samples_per_second = 48000;
	int bytes_per_sample = 4;
	int sound_buffer_size = samples_per_second * bytes_per_sample;
	int tone_hz = 256;
	int tone_amplitude = 500;
	int wave_period = samples_per_second / tone_hz;
	int half_wave_period = wave_period / 2;
	uint32_t running_sample_index = 0;
	InitDirectSound(hwnd, samples_per_second, sound_buffer_size, sound_buffer);
	sound_buffer->Play(0, 0, DSBPLAY_LOOPING);
	
	// init screen buffer
	//FillPlatformBitBuffer(shared_state.bitBuff, MakeColor(255, 255, 255, 255));
	
	// Frame timing
	LARGE_INTEGER prev_counter;
    uint64_t prev_cycle_counter = __rdtsc();
    QueryPerformanceCounter(&prev_counter);
	std::chrono::steady_clock::time_point currTime = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point prevTime{};

    // Run the message loop.
    MSG msg{};
	while (shared_state.is_running == 1) {
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
		
		
		// Timing
		prevTime = currTime;
		currTime = std::chrono::steady_clock::now();
		std::chrono::duration<float> dur = currTime - prevTime;
		float elapsedTime = dur.count();
		
		DWORD play_cursor;
        DWORD write_cursor;
		if(SUCCEEDED(sound_buffer->GetCurrentPosition(&play_cursor, &write_cursor)))
		{
			DWORD lock_offset = running_sample_index * bytes_per_sample % sound_buffer_size;
			void *region1;
			DWORD region1_size;
			void *region2;
			DWORD region2_size;
			DWORD bytes_to_lock;
	
			if(play_cursor == lock_offset) {
				bytes_to_lock = sound_buffer_size;
			} else if(lock_offset > play_cursor) {
				bytes_to_lock = sound_buffer_size - lock_offset + play_cursor;
			} else {
				bytes_to_lock = play_cursor - lock_offset;
			}
	
			if(SUCCEEDED(sound_buffer->Lock(
					lock_offset,
					bytes_to_lock,
					&region1, &region1_size,
					&region2, &region2_size,
					0
				)))
			{
				int16_t *sample_out = (int16_t*)region1;
				for(int i = 0; i < region1_size / bytes_per_sample; i++) {
					int16_t value = ((running_sample_index++ / half_wave_period) % 2) ? tone_amplitude : -tone_amplitude;
					*sample_out++ = value;
					*sample_out++ = value;
				}
		
				sample_out = (int16_t*)region2;
				for(int i = 0; i < region2_size / bytes_per_sample; i++) {
					int16_t value = ((running_sample_index++ / half_wave_period) % 2) ? tone_amplitude : -tone_amplitude;
					*sample_out++ = value;
					*sample_out++ = value;
				}
		
				sound_buffer->Unlock(region1, region1_size, region2, region2_size);
			}
        }
		
		UpdateProjectFunc(&shared_state);
		
		HDC hdc = GetDC(hwnd);
		((W32Extra*)(shared_state.extra))->hdc = hdc;
		PlatformUpdateDisplay(&shared_state, shared_state.client_width, shared_state.client_height);
		ReleaseDC(hwnd, hdc);
		
		uint64_t current_cycle_counter = __rdtsc();
        LARGE_INTEGER current_counter;
        QueryPerformanceCounter(&current_counter);

        int64_t counter_elapsed = current_counter.QuadPart - prev_counter.QuadPart;
        uint64_t cycles_elapsed = current_cycle_counter - prev_cycle_counter;

        float ms_per_frame = 1000.0f * (float)counter_elapsed / counter_per_sec.QuadPart;
        float fps = (float)counter_per_sec.QuadPart / counter_elapsed;
        float mega_cycles_per_frame = (float)cycles_elapsed / (1000.0f * 1000.0f);

        char Buffer[256];
        sprintf(Buffer, "ms/f: %.2f,  fps: %.2f,  mc/f: %.2f\n", ms_per_frame, fps, mega_cycles_per_frame);
        //OutputDebugStringA(Buffer);

        prev_counter = current_counter;
        prev_cycle_counter = current_cycle_counter;
		
		if (shared_state.max_fps > 0)
		{
			float sec_per_frame = 1.0f / shared_state.max_fps;
			if (elapsedTime < sec_per_frame)
				Sleep((int)((sec_per_frame-elapsedTime) * 1000.0f));
		}
	}

    return 0;
}
