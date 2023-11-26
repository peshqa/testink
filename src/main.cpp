/*
main.cpp - entry point
Console isn't used by the app (initially), so 'wWinMain' is the main function.
2023/08/10, peshqa
*/
#define UNICODE

#include "project.h"
#include "simple_wasapi_renderer.h"
#include "simple_win32_renderer.h"


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	srand(time(NULL));
	
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
    //ShowWindow(hwnd, nCmdShow);
	PlatformGoBorderlessFullscreen(&shared_state);
	
	// init screen buffer
	FillPlatformBitBuffer(shared_state.bitBuff, MakeColor(255, 255, 255, 255));
	
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
		
		// Input (TODO)
		
		// Update & Render

		//std::wstring title = std::to_wstring((elapsedTime));
		//SetWindowTextW(hwnd, title.c_str());
		
		UpdateProjectFunc(&shared_state);
		
		HDC hdc = GetDC(hwnd);
		((W32Extra*)(shared_state.extra))->hdc = hdc;
		PlatformUpdateDisplay(&shared_state, shared_state.client_width, shared_state.client_height);
		ReleaseDC(hwnd, hdc);
		
		Sleep(25); // ms
	}

    return 0;
}
