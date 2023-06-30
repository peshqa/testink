#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <stdio.h>

#pragma comment(lib, "Kernel32.lib")

int main()
{
	wchar_t title[] = L"LOL!";
	SetConsoleTitle(title);
	
	CreateConsoleScreenBuffer(
  _In_             DWORD               dwDesiredAccess,
  _In_             DWORD               dwShareMode,
  _In_opt_   const SECURITY_ATTRIBUTES *lpSecurityAttributes,
  _In_             DWORD               dwFlags,
  _Reserved_       LPVOID              lpScreenBufferData);
	
	system("pause");
	return 0;
}