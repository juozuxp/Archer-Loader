#pragma once

#undef CreateProcess

class ProcessManager
{
public:
	static bool FileExists(const char* FilePath);
	static bool PatchSteam(unsigned long ProcessID);
	static bool ResumeProcess(unsigned long ProcessID);
	static bool TerminateService(const char* ServiceName);
	static bool RestoreTheProcess(const char* ProcessPath);
	static bool WaitForProcessExit(const char* ProcessName);
	static bool WaitForProcessExit(unsigned long ProcessID);
	static bool WaitForProcessWindow(const char* WindowName);
	static bool WaitForProcessTermination(const char* ProcessName);
	static bool WaitForProcessTermination(unsigned long ProcessID);
	static bool WaitForProcess(const char* ProcessName, struct tagPROCESSENTRY32* Process);
	static bool GetImagePath(unsigned long ProcessID, char* Buffer, unsigned long BufferSize);
	static bool ReplaceTheProcess(const char* ProcessPath, void* Image, unsigned long ImageSize);
	static bool LoadDll(unsigned long ProcessID, void* Reserved, unsigned long ReservedSize, const void* DllBuffer, unsigned long DllSize);
};

