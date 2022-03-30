#include "Mutex.h"
#include "ExceptionHandler.h"
#include "ClientManager.h"
#include <Imports.h>
#include <minidumpapiset.h>

#include "CryptString.h"

#include <PEDisector.h>
#include <x86_x64Shell.h>

#ifndef _DEBUG
#include <VirtualizerSDK.h>
#endif

#pragma comment(lib, "Dbghelp.lib")

ExceptionHandler ExceptionHandler::Instance;

void ExceptionHandler::Initialize()
{
	IMAGE_SECTION_HEADER* TextSection;
	HMODULE Kernel32;

	unsigned long OldProtection;
	void* CodeLocation;

	Kernel32 = GetModuleHandleA("Kernel32.dll");
	if (!Kernel32)
		return;

	if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(FindSectionByName(Kernel32, ".text", &TextSection))))
		return;

	unsigned char ShellCode[] =
	{
		JMPQ_RM(REL_DO(0)),
		QWORDPTR(Handler)
	};

	CodeLocation = ((char*)Kernel32) + TextSection->VirtualAddress + TextSection->Misc.VirtualSize;

	if (!VirtualProtect(CodeLocation, sizeof(ShellCode), PAGE_EXECUTE_READWRITE, &OldProtection))
		return;

	memcpy(CodeLocation, ShellCode, sizeof(ShellCode));
	if (!VirtualProtect(CodeLocation, sizeof(ShellCode), OldProtection, &OldProtection))
		return;

	AddVectoredExceptionHandler(true, (PVECTORED_EXCEPTION_HANDLER)CodeLocation);
}

bool ExceptionHandler::Handleable(unsigned long ExceptionCode)
{


	switch (ExceptionCode)
	{
	case STATUS_ACCESS_VIOLATION:
	case STATUS_POSSIBLE_DEADLOCK:
	case STATUS_INSTRUCTION_MISALIGNMENT:
	case STATUS_DATATYPE_MISALIGNMENT:
	case STATUS_PRIVILEGED_INSTRUCTION:
	case STATUS_ILLEGAL_INSTRUCTION:
	case STATUS_BREAKPOINT:
	case STATUS_STACK_OVERFLOW:
	case STATUS_HANDLE_NOT_CLOSABLE:
	case STATUS_IN_PAGE_ERROR:
	case STATUS_ASSERTION_FAILURE:
	case STATUS_STACK_BUFFER_OVERRUN:
	case STATUS_GUARD_PAGE_VIOLATION:
	case STATUS_REG_NAT_CONSUMPTION:
	{
		return false;
	} break;
	}



	return true;
}

#pragma optimize("", off)
long ExceptionHandler::Handler(_EXCEPTION_POINTERS* ExceptionInfo)
{


	static Mutex ThreadLock = Mutex();

	unsigned long Length;
	char Path[MAX_PATH];

	void* CrashBuffer;

	MINIDUMP_EXCEPTION_INFORMATION ExceptionParams;
	OVERLAPPED Overlapped;
	HANDLE ExceptionFile;
	ArcherType Type;

	if (Handleable(ExceptionInfo->ExceptionRecord->ExceptionCode))
		return EXCEPTION_CONTINUE_SEARCH;

	if (!ThreadLock.Lock())
		TerminateThread(GetCurrentThread(), 0);

	Length = GetTempPathA(sizeof(Path), Path);
	if (!Length)
		exit(0);

	memcpy(Path + Length, __CS("ASFNSAFNAJFNSJUFISANFAJSF.dmp"), sizeof("ASFNSAFNAJFNSJUFISANFAJSF.dmp"));

	ExceptionFile = CreateFileA(Path, GENERIC_WRITE | GENERIC_READ, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, 0);
	if (ExceptionFile == INVALID_HANDLE_VALUE)
	{
		unsigned long Error = GetLastError();
		exit(0);
	}

	ExceptionParams.ClientPointers = TRUE;
	ExceptionParams.ThreadId = GetCurrentThreadId();
	ExceptionParams.ExceptionPointers = ExceptionInfo;
	if (!MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), ExceptionFile, MiniDumpNormal, &ExceptionParams, 0, 0))
	{
		CloseHandle(ExceptionFile);
		exit(0);
	}

	Length = GetFileSize(ExceptionFile, 0);
	if (!Length)
	{
		CloseHandle(ExceptionFile);
		exit(0);
	}

	CrashBuffer = malloc(Length);
	if (!CrashBuffer)
	{
		CloseHandle(ExceptionFile);
		exit(0);
	}

	memset(&Overlapped, 0, sizeof(Overlapped));
	if (!ReadFile(ExceptionFile, CrashBuffer, Length, 0, &Overlapped))
	{
		CloseHandle(ExceptionFile);
		free(CrashBuffer);
		exit(0);
	}

	if (!ClientManager::Send(ArcherType_CrashDump, &Length, sizeof(Length)))
	{
		CloseHandle(ExceptionFile);
		free(CrashBuffer);
		exit(0);
	}

	if (!ClientManager::Send(ArcherType_CrashDump, CrashBuffer, Length))
	{
		CloseHandle(ExceptionFile);
		free(CrashBuffer);
		exit(0);
	}

	Type = ArcherType_None;
	while (Type != ArcherType_CrashDump)
	{
		Type = ClientManager::Receive(0, 0, 0);
		if (Type == ArcherType_Disconnect)
		{
			CloseHandle(ExceptionFile);
			DeleteFileA(Path);
			exit(0);
		}
	}

	CloseHandle(ExceptionFile);
	free(CrashBuffer);



	exit(0);
}
#pragma optimize("", on)