#include "ExtensionManager.h"
#include <Imports.h>
#include <PEDisector.h>
#include <PEMapper.h>

#ifndef _DEBUG
#include <VirtualizerSDK.h>
#endif

unsigned long long ExtensionManager::GetAddressOfModule(const char* PEName, void** PEBuffer, void* Reserved)
{


	*PEBuffer = LoadLibraryA(PEName);



	return 0;
}

unsigned long long ExtensionManager::GetExportOfModule(void* PEBuffer, const char* Export, void** ExportAddress, void* Reserved)
{


	*ExportAddress = GetProcAddress((HMODULE)PEBuffer, Export);



	return 0;
}

bool ExtensionManager::ExtendTheProcess(void* Image, unsigned long long ImageSize, void* Reserved)
{


	void* ImageBuffer;

	ImageBuffer = VirtualAlloc(0, GET_IMAGE_OPTIONAL_HEADER(Image)->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!ImageBuffer)
		return false;

	if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(MapImageSections(Image, ImageBuffer))))
	{
		VirtualFree(ImageBuffer, 0, MEM_RELEASE);
		return false;
	}

	if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(RelocateImage(Image, ImageBuffer, ImageBuffer))))
	{
		VirtualFree(ImageBuffer, 0, MEM_RELEASE);
		return false;
	}

	if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(FixImageImports(Image, ImageBuffer, { GetAddressOfModule, 0 }, { GetExportOfModule, 0 }))))
	{
		VirtualFree(ImageBuffer, 0, MEM_RELEASE);
		return false;
	}

	((BOOLEAN(*)(void*, unsigned long, void*))(((unsigned char*)ImageBuffer) + GET_IMAGE_OPTIONAL_HEADER(Image)->AddressOfEntryPoint))(ImageBuffer, DLL_PROCESS_ATTACH, Reserved);


}