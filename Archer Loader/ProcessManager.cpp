#include "ProcessManager.h"
#include <Imports.h>
#include <PEDisector.h>
#include <BasicUtilities.h>
#include <PEMapper.h>

#include "CryptString.h"

#define SHELL_RELATIVITY MainRelativity

#include <x86_x64Shell.h>

#ifndef _DEBUG
#include <ThemidaSDK.h>
#endif

unsigned long long ProcessManager::GetAddressOfModule(const char* PEName, void** PEBuffer, void* Reserved)
{


	*PEBuffer = LoadLibraryA(PEName);



	return 0;
}

unsigned long long ProcessManager::GetExportOfModule(void* PEBuffer, const char* Export, void** ExportAddress, void* Reserved)
{


	*ExportAddress = GetProcAddress((HMODULE)PEBuffer, Export);



	return 0;
}

bool ProcessManager::ExecuteDll(const void* DllBuffer, unsigned long DllSize, void* Reserved)
{


	void* ImageBuffer;

	ImageBuffer = VirtualAlloc(0, GET_IMAGE_OPTIONAL_HEADER(DllBuffer)->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!ImageBuffer)
		return false;

	if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(MapImageSections(DllBuffer, ImageBuffer))))
	{
		VirtualFree(ImageBuffer, 0, MEM_RELEASE);
		return false;
	}

	if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(RelocateImage(DllBuffer, ImageBuffer, ImageBuffer))))
	{
		VirtualFree(ImageBuffer, 0, MEM_RELEASE);
		return false;
	}

	if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(FixImageImports(DllBuffer, ImageBuffer, { GetAddressOfModule, 0 }, { GetExportOfModule, 0 }))))
	{
		VirtualFree(ImageBuffer, 0, MEM_RELEASE);
		return false;
	}

	((BOOLEAN(*)(void*, unsigned long, void*))(((unsigned char*)ImageBuffer) + GET_IMAGE_OPTIONAL_HEADER(DllBuffer)->AddressOfEntryPoint))(ImageBuffer, DLL_PROCESS_ATTACH, Reserved);



	return true;
}