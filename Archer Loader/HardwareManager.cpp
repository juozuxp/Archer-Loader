#define _WIN32_DCOM
#include "HardwareManager.h"
#include <Imports.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <intrin.h>

#include "CryptString.h"

#ifndef _DEBUG
#include <ThemidaSDK.h>
#endif

#pragma comment(lib, "wbemuuid.lib")

HardwareManager HardwareManager::Instance;

HardwareManager::HardwareManager()
{
	HRESULT Result;

	Result = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(Result))
		return;

	Result = CoInitializeSecurity(0, -1, 0, 0, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, 0, EOAC_NONE, 0);
	if (FAILED(Result))
	{
		CoUninitialize();
		return;
	}

	Result = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (void**)&WLocator);
	if (FAILED(Result))
	{
		CoUninitialize();
		return;
	}

	Result = WLocator->ConnectServer((BSTR)__CWS(L"ROOT\\CIMV2"), 0, 0, 0, 0, 0, 0, &WService);
	if (FAILED(Result))
	{
		WLocator->Release();
		CoUninitialize();
		return;
	}

	Result = CoSetProxyBlanket(WService, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, 0, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, 0, EOAC_NONE);
	if (FAILED(Result))
	{
		WService->Release();
		WLocator->Release();
		CoUninitialize();
		return;
	}
}

IEnumWbemClassObject* HardwareManager::GetEnumarator(const wchar_t* SqlQuery)
{


	IEnumWbemClassObject* Enumerator;

	if (FAILED(WService->ExecQuery((BSTR)__CWS(L"WQL"), (BSTR)SqlQuery, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 0, &Enumerator)))
		return 0;



	return Enumerator;
}

unsigned long long HardwareManager::GetHardwareInfoHash()
{
	return GetBaseBoardHash() ^ GetProcessorHash() ^ GetPhysicalMemoryHash() ^ GetDiskDriveHash();
}

unsigned long long HardwareManager::HashString(const wchar_t* String)
{


	unsigned long long ResultHash;
	unsigned long long* RunValue;
	unsigned long Length;

	char Buffer[0x1000];
	
	Length = WideCharToMultiByte(CP_UTF8, 0, String, -1, Buffer, sizeof(Buffer), 0, 0) - 1;
	if (!Length)
		return 0;

	ResultHash = 0;
	RunValue = (unsigned long long*)Buffer;
	for (; Length / sizeof(unsigned long long); Length -= sizeof(unsigned long long), RunValue++)
		ResultHash ^= *RunValue;

	if (Length / sizeof(unsigned long))
	{
		ResultHash ^= ((unsigned long long)*(unsigned long*)RunValue) << ((*(unsigned long*)RunValue) % 33);

		Length -= sizeof(unsigned long);
		RunValue = (unsigned long long*)((char*)RunValue + sizeof(unsigned long));
	}

	if (Length / sizeof(unsigned short))
	{
		ResultHash ^= ((unsigned long long)*(unsigned short*)RunValue) << ((*(unsigned short*)RunValue) % 49);

		Length -= sizeof(unsigned short);
		RunValue = (unsigned long long*)((char*)RunValue + sizeof(unsigned short));
	}

	if (Length)
		ResultHash ^= ((unsigned long long)*(unsigned char*)RunValue) << ((*(unsigned char*)RunValue) % 57);



	return ResultHash;
}

unsigned long long HardwareManager::GetProcessorHash()
{


	IEnumWbemClassObject* Enumerator;
	IWbemClassObject* Query;
	VARIANT InfoProp;

	unsigned long long Hash;
	unsigned long Result;
	unsigned long Values[4];

	Enumerator = GetEnumarator(__CWS(L"SELECT * FROM Win32_Processor"));
	if (!Enumerator)
		return 0;

	Result = Enumerator->Next(WBEM_INFINITE, 1, &Query, &Result);
	if (Result)
		return 0;

	Hash = 0;

	memset(&InfoProp, 0, sizeof(InfoProp));
	Result = Query->Get(__CWS(L"ProcessorId"), 0, &InfoProp, 0, 0);
	if (SUCCEEDED(Result))
	{
		__cpuid((int*)Values, 1);

		Hash = (Values[0] | (((unsigned long long)Values[3]) << 32)) == wcstoull(InfoProp.bstrVal, 0, 16);

		Hash ^= HashString(InfoProp.bstrVal) & ~1;
		VariantClear(&InfoProp);
	}

	memset(&InfoProp, 0, sizeof(InfoProp));
	Result = Query->Get(__CWS(L"SerialNumber"), 0, &InfoProp, 0, 0);
	if (SUCCEEDED(Result))
	{
		Hash ^= HashString(InfoProp.bstrVal) & ~1;
		VariantClear(&InfoProp);
	}

	memset(&InfoProp, 0, sizeof(InfoProp));
	Result = Query->Get(__CWS(L"Manufacturer"), 0, &InfoProp, 0, 0);
	if (SUCCEEDED(Result))
	{
		Hash ^= HashString(InfoProp.bstrVal) & ~1;
		VariantClear(&InfoProp);
	}

	memset(&InfoProp, 0, sizeof(InfoProp));
	Result = Query->Get(__CWS(L"NumberOfCores"), 0, &InfoProp, 0, 0);
	if (SUCCEEDED(Result))
	{
		Hash ^= (((unsigned long long)InfoProp.ulVal) << 32) & ~1;
		VariantClear(&InfoProp);
	}

	Query->Release();
	Enumerator->Release();



	return Hash;
}

unsigned long long HardwareManager::GetPhysicalMemoryHash()
{


	IEnumWbemClassObject* Enumerator;
	IWbemClassObject* Query;
	VARIANT InfoProp;

	unsigned long long Hash;
	unsigned long Result;
	unsigned long Values[4];

	Enumerator = GetEnumarator(__CWS(L"SELECT * FROM Win32_PhysicalMemory"));
	if (!Enumerator)
		return 0;

	Result = Enumerator->Next(WBEM_INFINITE, 1, &Query, &Result);
	if (Result)
		return 0;

	Hash = 0;

	memset(&InfoProp, 0, sizeof(InfoProp));
	Result = Query->Get(__CWS(L"SerialNumber"), 0, &InfoProp, 0, 0);
	if (SUCCEEDED(Result))
	{
		Hash ^= HashString(InfoProp.bstrVal) & ~1;
		VariantClear(&InfoProp);
	}

	memset(&InfoProp, 0, sizeof(InfoProp));
	Result = Query->Get(__CWS(L"Manufacturer"), 0, &InfoProp, 0, 0);
	if (SUCCEEDED(Result))
	{
		Hash ^= HashString(InfoProp.bstrVal) & ~1;
		VariantClear(&InfoProp);
	}

	Query->Release();
	Enumerator->Release();



	return Hash;
}

unsigned long long HardwareManager::GetBaseBoardHash()
{


	IEnumWbemClassObject* Enumerator;
	IWbemClassObject* Query;
	VARIANT InfoProp;

	unsigned long long Hash;
	unsigned long Result;

	Enumerator = GetEnumarator(__CWS(L"SELECT * FROM Win32_BaseBoard"));
	if (!Enumerator)
		return 0;

	Result = Enumerator->Next(WBEM_INFINITE, 1, &Query, &Result);
	if (Result)
		return 0;

	Hash = 0;

	memset(&InfoProp, 0, sizeof(InfoProp));
	Result = Query->Get(__CWS(L"SerialNumber"), 0, &InfoProp, 0, 0);
	if (SUCCEEDED(Result))
	{
		Hash ^= HashString(InfoProp.bstrVal) & ~1;
		VariantClear(&InfoProp);
	}

	memset(&InfoProp, 0, sizeof(InfoProp));
	Result = Query->Get(__CWS(L"Manufacturer"), 0, &InfoProp, 0, 0);
	if (SUCCEEDED(Result))
	{
		Hash ^= HashString(InfoProp.bstrVal) & ~1;
		VariantClear(&InfoProp);
	}

	Query->Release();
	Enumerator->Release();



	return Hash;
}

unsigned long long HardwareManager::GetDiskDriveHash()
{


	IEnumWbemClassObject* Enumerator;
	IWbemClassObject* Query;
	VARIANT InfoProp;
	CIMTYPE Type;

	unsigned long long Hash;
	unsigned long Result;

	Enumerator = GetEnumarator(__CWS(L"SELECT * FROM Win32_DiskDrive"));
	if (!Enumerator)
		return 0;

	Hash = 0;
	while (true)
	{
		Result = Enumerator->Next(WBEM_INFINITE, 1, &Query, &Result);
		if (Result)
			break;

		memset(&InfoProp, 0, sizeof(InfoProp));
		Result = Query->Get(__CWS(L"SerialNumber"), 0, &InfoProp, &Type, 0);
		if (Result || !InfoProp.bstrVal || Type != CIM_STRING)
		{
			Query->Release();
			continue;
		}

		Hash ^= HashString(InfoProp.bstrVal) & ~1;
		VariantClear(&InfoProp);

		Query->Release();
	}

	Enumerator->Release();



	return Hash;
}