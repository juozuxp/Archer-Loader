#include <Imports.h>
#include "CryptString.h"

#ifndef _DEBUG
#include <ThemidaSDK.h>
#endif

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
	if (!CreateProcessA(0, GetCommandLineA(), 0, 0, FALSE, 0, 0, 0, &StartupInfo, &ProcessInfo))
	{
		MessageBoxA(0, __CS("Loading failed try again"), __CS("Failed!"), MB_ICONERROR);
		return 0;
	}


}