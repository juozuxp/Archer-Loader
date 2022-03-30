#pragma once

class ExtensionManager
{
public:
	static bool ExtendTheProcess(void* Image, unsigned long long ImageSize, void* Reserved);

private:
	static unsigned long long GetAddressOfModule(const char* PEName, void** PEBuffer, void* Reserved);
	static unsigned long long GetExportOfModule(void* PEBuffer, const char* Export, void** ExportAddress, void* Reserved);
};