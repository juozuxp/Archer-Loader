#include <Imports.h>
#include <PEDisector.h>
#include <Utilities.h>
#include <WindowsEntrys.h>

#define SHELL_RELATIVITY MainRelativity 
#include <x86_x64Shell.h>

#include "CryptString.h"

#ifndef _DEBUG
#include <ThemidaSDK.h>
#endif

unsigned long ConstructCurrentDirectoryA(char* Path, unsigned long Size)
{
	unsigned long Length;

	Length = GetModuleFileNameA(0, Path, Size);

	for (Path += Length; Length; Path--, Length--)
	{
		if (*Path == '\\')
			break;
	}

	*Path = '\0';

	return Length;
}

unsigned long ConstructCurrentDirectoryW(wchar_t* Path, unsigned long Size)
{
	unsigned long Length;

	Length = GetModuleFileNameW(0, Path, Size);
	for (Path += Length; Length; Path--, Length--)
	{
		if (*Path == L'\\')
			break;
	}

	*Path = L'\0';

	return Length;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{


	PROCESS_INFORMATION ProcessInfo;
	STARTUPINFOA StartupInfo;

	char* LocationBuffer;

	LocationBuffer = strstr(GetCommandLineA(), __CS("_BE.exe"));
	memmove(LocationBuffer, LocationBuffer + sizeof("_BE") - 1, strlen(LocationBuffer + sizeof("_BE") - 1) + 1);

	memset(&ProcessInfo, 0, sizeof(ProcessInfo));
	memset(&StartupInfo, 0, sizeof(StartupInfo));

	StartupInfo.cb = sizeof(StartupInfo);
	if (!CreateProcessA(0, GetCommandLineA(), 0, 0, FALSE, CREATE_SUSPENDED, 0, 0, &StartupInfo, &ProcessInfo))
	{
		MessageBoxA(0, __CS("Loading failed try again"), __CS("Failed! 0"), MB_ICONERROR);
		return 0;
	}


}