#pragma once

class ProcessManager
{
public:
	static bool ExecuteDll(const void* DllBuffer, unsigned long DllSize, void* Reserved);

private:
	static unsigned long long GetAddressOfModule(const char* PEName, void** PEBuffer, void* Reserved);
	static unsigned long long GetExportOfModule(void* PEBuffer, const char* Export, void** ExportAddress, void* Reserved);
};

