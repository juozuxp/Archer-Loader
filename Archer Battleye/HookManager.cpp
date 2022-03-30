#include "HookManager.h"
#include "BattleyeManager.h"

#include "CryptString.h"

#ifndef _DEBUG
#include <VirtualizerSDK.h>
#endif

HookManager HookManager::Instance;

void HookManager::Initialize()
{


	HookLibrary();
	HookGetProc();
	HookService();
	HookDeleteFile();


}

void HookManager::HookLibrary()
{


	MappedImportDescriptor Import;
	GeneralError Error;
	HMODULE MainModule;

	MainModule = GetModuleHandleA(0);

	Error.ErrorValue = FindImportByNameMapped(MainModule, __CS("LoadLibraryA"), &Import);
	if (NT_SUCCESS(Error.NTStatus))
		PatchImport(Import.FirstThunk, HookedLoadLibraryA);

	Error.ErrorValue = FindImportByNameMapped(MainModule, __CS("LoadLibraryW"), &Import);
	if (NT_SUCCESS(Error.NTStatus))
		PatchImport(Import.FirstThunk, HookedLoadLibraryW);

	Error.ErrorValue = FindImportByNameMapped(MainModule, __CS("LoadLibraryExA"), &Import);
	if (NT_SUCCESS(Error.NTStatus))
		PatchImport(Import.FirstThunk, HookedLoadLibraryExA);

	Error.ErrorValue = FindImportByNameMapped(MainModule, __CS("LoadLibraryExW"), &Import);
	if (NT_SUCCESS(Error.NTStatus))
		PatchImport(Import.FirstThunk, HookedLoadLibraryExW);

	Error.ErrorValue = FindImportByNameMapped(MainModule, __CS("FreeLibrary"), &Import);
	if (NT_SUCCESS(Error.NTStatus))
		PatchImport(Import.FirstThunk, HookedFreeLibrary);


}

void HookManager::HookGetProc()
{
	MappedImportDescriptor Import;
	GeneralError Error;
	HMODULE MainModule;

	MainModule = GetModuleHandleA(0);

	Error.ErrorValue = FindImportByNameMapped(MainModule, __CS("GetProcAddress"), &Import);
	if (NT_SUCCESS(Error.NTStatus))
		PatchImport(Import.FirstThunk, HookedGetProcAddress);
}

void HookManager::HookService()
{


	MappedImportDescriptor Import;
	GeneralError Error;
	HMODULE MainModule;

	MainModule = GetModuleHandleA(0);

	Error.ErrorValue = FindImportByNameMapped(MainModule, __CS("OpenServiceA"), &Import);
	if (NT_SUCCESS(Error.NTStatus))
		PatchImport(Import.FirstThunk, HookedOpenServiceA);

	Error.ErrorValue = FindImportByNameMapped(MainModule, __CS("OpenServiceW"), &Import);
	if (NT_SUCCESS(Error.NTStatus))
		PatchImport(Import.FirstThunk, HookedOpenServiceW);

	Error.ErrorValue = FindImportByNameMapped(MainModule, __CS("QueryServiceStatus"), &Import);
	if (NT_SUCCESS(Error.NTStatus))
		PatchImport(Import.FirstThunk, HookedQueryServiceStatus);


}

void HookManager::HookDeleteFile()
{
	MappedImportDescriptor Import;
	GeneralError Error;
	HMODULE MainModule;

	MainModule = GetModuleHandleA(0);

	Error.ErrorValue = FindImportByNameMapped(MainModule, __CS("DeleteFileA"), &Import);
	if (NT_SUCCESS(Error.NTStatus))
		PatchImport(Import.FirstThunk, HookedDeleteFileA);

	Error.ErrorValue = FindImportByNameMapped(MainModule, __CS("DeleteFileW"), &Import);
	if (NT_SUCCESS(Error.NTStatus))
		PatchImport(Import.FirstThunk, HookedDeleteFileW);
}

void HookManager::PatchImport(void** Thunk, void* Function)
{
	unsigned long OldProtection;

	VirtualProtect(Thunk, sizeof(Thunk), PAGE_EXECUTE_READWRITE, &OldProtection);
	*Thunk = Function;
	VirtualProtect(Thunk, sizeof(Thunk), OldProtection, &OldProtection);
}

SC_HANDLE HookManager::HookedOpenServiceA(SC_HANDLE Handle, const char* ServiceName, unsigned long DesiredAccess)
{


	if (strstr(ServiceName, __CS("BEService")))
		return (SC_HANDLE)0xDEADBE;



	return OpenServiceA(Handle, ServiceName, DesiredAccess);
}

SC_HANDLE HookManager::HookedOpenServiceW(SC_HANDLE Handle, const wchar_t* ServiceName, unsigned long DesiredAccess)
{


	if (wcsstr(ServiceName, __CWS(L"BEService")))
		return (SC_HANDLE)0xDEADBE;



	return OpenServiceW(Handle, ServiceName, DesiredAccess);
}

BOOL HookManager::HookedQueryServiceStatus(SC_HANDLE Handle, LPSERVICE_STATUS Status)
{


	if (Handle == (SC_HANDLE)0xDEADBE)
	{
		Status->dwServiceType = SERVICE_KERNEL_DRIVER;
		Status->dwCurrentState = SERVICE_RUNNING;
		Status->dwControlsAccepted = SERVICE_ACCEPT_STOP;
		Status->dwWin32ExitCode = NO_ERROR;
		Status->dwServiceSpecificExitCode = 0;
		Status->dwCheckPoint = 0;
		Status->dwWaitHint = 0;

		return TRUE;
	}



	return QueryServiceStatus(Handle, Status);
}

BOOL HookManager::HookedDeleteFileA(const char* Path)
{


	if (strstr(Path, __CS("BEClient_x64.dll")))
		return TRUE;



	return DeleteFileA(Path);
}

BOOL HookManager::HookedDeleteFileW(const wchar_t* Path)
{


	if (wcsstr(Path, __CWS(L"BEClient_x64.dll")))
		return TRUE;



	return DeleteFileW(Path);
}

void* HookManager::HookedGetProcAddress(HMODULE Module, const char* API)
{


	if (Module != ((HMODULE)0xDEADBE))
		return GetProcAddress(Module, API);

	if (!stricmp(API, __CS("Init")))
		return BattleyeManager::Init;

	if (!stricmp(API, __CS("GetVer")))
		return BattleyeManager::GetVer;



	return 0;
}

HMODULE HookManager::HookedLoadLibraryA(const char* Library)
{


	if (strstr(Library, __CS("BEClient_x64.dll")))
		return ((HMODULE)0xDEADBE);



	return LoadLibraryA(Library);
}

HMODULE HookManager::HookedLoadLibraryW(const wchar_t* Library)
{


	if (wcsstr(Library, __CWS(L"BEClient_x64.dll")))
		return ((HMODULE)0xDEADBE);



	return LoadLibraryW(Library);
}

HMODULE HookManager::HookedLoadLibraryExA(const char* Library, HANDLE File, unsigned long Flags)
{


	if (strstr(Library, __CS("BEClient_x64.dll")))
		return ((HMODULE)0xDEADBE);



	return LoadLibraryExA(Library, File, Flags);
}

HMODULE HookManager::HookedLoadLibraryExW(const wchar_t* Library, HANDLE File, unsigned long Flags)
{


	if (wcsstr(Library, __CWS(L"BEClient_x64.dll")))
		return ((HMODULE)0xDEADBE);



	return LoadLibraryExW(Library, File, Flags);
}

BOOL HookManager::HookedFreeLibrary(HMODULE Module)
{


	if (Module == ((HMODULE)0xDEADBE))
		return TRUE;



	return FreeLibrary(Module);
}