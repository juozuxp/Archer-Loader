#pragma once

class HardwareManager
{
public:
	HardwareManager();

public:
	unsigned long long GetHardwareInfoHash();

public:
	static HardwareManager Instance;

private:
	unsigned long long GetDiskDriveHash();
	unsigned long long GetBaseBoardHash();
	unsigned long long GetProcessorHash();
	unsigned long long GetPhysicalMemoryHash();

	unsigned long long HashString(const wchar_t* String);

	class IEnumWbemClassObject* GetEnumarator(const wchar_t* SqlQuery);

private:
	class IWbemLocator* WLocator;
	class IWbemServices* WService;
};

