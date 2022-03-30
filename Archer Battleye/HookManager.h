#pragma once
#include <Imports.h>
#include <PEDisector.h>

class HookManager
{
public:
	void Initialize();

public:
	static HookManager Instance;

private:


private:
	void HookLibrary();
	void HookGetProc();
	void HookService();
	void HookDeleteFile();

	void PatchImport(void** Thunk, void* Function);
	
	static void* HookedGetProcAddress(HMODULE Module, const char* API);
	static BOOL HookedQueryServiceStatus(SC_HANDLE Handle, LPSERVICE_STATUS Status);

	static BOOL HookedFreeLibrary(HMODULE Module);
	static HMODULE HookedLoadLibraryA(const char* Library);
	static HMODULE HookedLoadLibraryW(const wchar_t* Library);
	static HMODULE HookedLoadLibraryExA(const char* Library, HANDLE File, unsigned long Flags);
	static HMODULE HookedLoadLibraryExW(const wchar_t* Library, HANDLE File, unsigned long Flags);
	
	static SC_HANDLE HookedOpenServiceA(SC_HANDLE Handle, const char* ServiceName, unsigned long DesiredAccess);
	static SC_HANDLE HookedOpenServiceW(SC_HANDLE Handle, const wchar_t* ServiceName, unsigned long DesiredAccess);

	static BOOL HookedDeleteFileA(const char* Path);
	static BOOL HookedDeleteFileW(const wchar_t* Path);
};