#include "ProcessManager.h"
#include <Imports.h>
#include <PEDisector.h>
#include <Utilities.h>
#include <BasicUtilities.h>
#include <stdio.h>
#include <PEMapper.h>

#include "CryptString.h"

#define SHELL_RELATIVITY MainRelativity

#include <x86_x64Shell.h>

#ifndef _DEBUG
#include <VirtualizerSDK.h>
#endif

#undef CreateProcess

bool ProcessManager::FileExists(const char* FilePath)
{
	return CreateFileA(FilePath, 0, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0) != INVALID_HANDLE_VALUE;
}

bool ProcessManager::ResumeProcess(unsigned long ProcessID)
{
	HANDLE ProcessHandle;

	ProcessHandle = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, ProcessID);
	if (ProcessHandle == INVALID_HANDLE_VALUE)
		return false;

	if (!NT_SUCCESS(NtResumeProcess(ProcessHandle)))
		return false;

	CloseHandle(ProcessHandle);
	return true;
}

bool ProcessManager::PatchSteam(unsigned long ProcessID)
{
	UnMappedImportDescriptor UnMappedImport;
	IMAGE_SECTION_HEADER* KernelText;
	PROCESSENTRY32 Process;
	MODULEENTRY32 Module;
	GeneralError Error;
	BOOLEAN Enabled;

	HANDLE ProcessHandle;
	HANDLE FileHandle;

	unsigned long MainRelativity;
	unsigned long DontIgnoreCP;

	void* OpenProcessImport;
	void* ImportLocation;
	void* ShellLocation;
	void* FileDump;

	Error.ErrorValue = FindProcessByNameA(__CS("SteamService.exe"), &Process);
	if (NT_SUCCESS(Error.NTStatus))
	{
		if (!NT_SUCCESS(RtlAdjustPrivilege(SeDebugPrivilege, TRUE, FALSE, &Enabled)))
		{
			MessageBoxA(0, __CS("Loading failed, launch the loader as admin and try again"), __CS("Failed! 1"), MB_ICONERROR);
			return false;
		}

		Error.ErrorValue = FindProcessModuleByNameA(Process.th32ProcessID, __CS("Kernel32.dll"), &Module);
		if (!NT_SUCCESS(Error.NTStatus))
		{
			MessageBoxA(0, __CS("Loading failed, launch the loader as admin and try again"), __CS("Failed! 2"), MB_ICONERROR);
			return false;
		}

		ProcessHandle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, Process.th32ProcessID);
		if (ProcessHandle == INVALID_HANDLE_VALUE)
		{
			MessageBoxA(0, __CS("Loading failed, launch the loader as admin and try again"), __CS("Failed! 3"), MB_ICONERROR);
			return false;
		}

		FileHandle = CreateFileA(__CS("C:\\Windows\\SysWOW64\\Kernel32.dll"), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (FileHandle == INVALID_HANDLE_VALUE)
		{
			MessageBoxA(0, __CS("Loading failed, launch the loader as admin and try again"), __CS("Failed! 4"), MB_ICONERROR);
			return false;
		}

		MainRelativity = GetFileSize(FileHandle, 0);
		if (!MainRelativity)
		{
			MessageBoxA(0, __CS("Loading failed, launch the loader as admin and try again"), __CS("Failed! 5"), MB_ICONERROR);
			return false;
		}

		FileDump = malloc(MainRelativity);
		if (!FileDump)
		{
			MessageBoxA(0, __CS("Loading failed try again"), __CS("Failed! 6"), MB_ICONERROR);
			return false;
		}

		if (!ReadFile(FileHandle, FileDump, MainRelativity, 0, 0))
		{
			MessageBoxA(0, __CS("Loading failed, launch the loader as admin and try again"), __CS("Failed! 7"), MB_ICONERROR);
			return false;
		}

		CloseHandle(FileHandle);

		Error.ErrorValue = FindSectionByName(FileDump, __CS(".text"), &KernelText);
		if (NT_SUCCESS(Error.NTStatus))
		{
			Error.ErrorValue = FindImportByNameUnMapped(FileDump, __CS("OpenProcess"), &UnMappedImport);
			if (NT_SUCCESS(Error.NTStatus))
			{
				ImportLocation = ((char*)Module.hModule) + UnMappedImport.FirstThunk;

				OpenProcessImport = 0;
				if (!ReadProcessMemory(ProcessHandle, ImportLocation, &OpenProcessImport, sizeof(unsigned long), 0))
				{
					MessageBoxA(0, __CS("Loading failed, launch the loader as admin and try again"), __CS("Failed! 8"), MB_ICONERROR);
					return false;
				}

				ShellLocation = ((char*)Module.hModule) + KernelText->VirtualAddress + KernelText->Misc.VirtualSize;
				if (OpenProcessImport == ShellLocation)
				{
					DontIgnoreCP = 0;
					MainRelativity = 0;
					D_C_S(unsigned, Shell, ,
						CMP_RM_D(MRSP_BO(0xC), ProcessID)
					);

					if (!VirtualProtectEx(ProcessHandle, ShellLocation, sizeof(Shell), PAGE_EXECUTE_READWRITE, &MainRelativity))
					{
						MessageBoxA(0, __CS("Loading failed, launch the loader as admin and try again"), __CS("Failed! 9"), MB_ICONERROR);
						return false;
					}

					if (!WriteProcessMemory(ProcessHandle, ShellLocation, Shell, sizeof(Shell), 0))
					{
						MessageBoxA(0, __CS("Loading failed, launch the loader as admin and try again"), __CS("Failed! 10"), MB_ICONERROR);
						return false;
					}

					VirtualProtectEx(ProcessHandle, ShellLocation, sizeof(Shell), MainRelativity, &MainRelativity);
				}
				else
				{
					DontIgnoreCP = 0;
					MainRelativity = 0;
					D_C_S(unsigned, Shell, ,
						CMP_RM_D(MRSP_BO(0xC), ProcessID),
						JNE_RB(DontIgnoreCP - MainRelativity),
						XORD_RM_R(LR_R(EAX, EAX)),
						NOTD_RM(LR(EAX)),
						RETN_W(0xC),
						R_CP(DontIgnoreCP, MOV_R_D(EAX, OpenProcessImport)),
						JMPQ_RM(LR(EAX))
					);

					ShellLocation = ((char*)Module.hModule) + KernelText->VirtualAddress + KernelText->Misc.VirtualSize;
					if (!VirtualProtectEx(ProcessHandle, ShellLocation, sizeof(Shell), PAGE_EXECUTE_READWRITE, &MainRelativity))
					{
						MessageBoxA(0, __CS("Loading failed, launch the loader as admin and try again"), __CS("Failed! 11"), MB_ICONERROR);
						return false;
					}

					if (!WriteProcessMemory(ProcessHandle, ShellLocation, Shell, sizeof(Shell), 0))
					{
						MessageBoxA(0, __CS("Loading failed, launch the loader as admin and try again"), __CS("Failed! 12"), MB_ICONERROR);
						return false;
					}

					VirtualProtectEx(ProcessHandle, ShellLocation, sizeof(Shell), MainRelativity, &MainRelativity);

					if (!VirtualProtectEx(ProcessHandle, ImportLocation, sizeof(unsigned long), PAGE_EXECUTE_READWRITE, &MainRelativity))
					{
						TerminateProcess(ProcessHandle, 0);

						MessageBoxA(0, __CS("Loading failed, launch the loader as admin and try again"), __CS("Failed! 13"), MB_ICONERROR);
						return false;
					}

					if (!WriteProcessMemory(ProcessHandle, ImportLocation, &ShellLocation, sizeof(unsigned long), 0))
					{
						TerminateProcess(ProcessHandle, 0);

						MessageBoxA(0, __CS("Loading failed, launch the loader as admin and try again"), __CS("Failed! 14"), MB_ICONERROR);
						return false;
					}

					VirtualProtectEx(ProcessHandle, ImportLocation, sizeof(unsigned long), MainRelativity, &MainRelativity);
				}
			}
		}
	}

	return true;
}

bool ProcessManager::WaitForProcess(const char* ProcessName, PROCESSENTRY32* Process)
{
	GeneralError Error;

	Error.ErrorValue = FindProcessByNameA(ProcessName, Process);
	if (NT_SUCCESS(Error.NTStatus))
		return true;

	while (!NT_SUCCESS(Error.NTStatus))
	{
		Error.ErrorValue = FindProcessByNameA(ProcessName, Process);
		Sleep(500);
	}

	return true;
}

bool ProcessManager::TerminateService(const char* ServiceName)
{


	SERVICE_STATUS ServiceStatus;
	SC_HANDLE ServiceManager;
	SC_HANDLE SerivceHandle;

	ServiceManager = OpenSCManagerA(0, 0, SC_MANAGER_ENUMERATE_SERVICE);
	if (!ServiceManager)
		return false;

	SerivceHandle = OpenServiceA(ServiceManager, ServiceName, SERVICE_STOP | SERVICE_QUERY_STATUS);
	if (!SerivceHandle)
	{
		CloseServiceHandle(ServiceManager);
		return false;
	}

	CloseServiceHandle(ServiceManager);

	if (!QueryServiceStatus(SerivceHandle, &ServiceStatus))
	{
		CloseServiceHandle(SerivceHandle);
		return false;
	}

	if (ServiceStatus.dwCurrentState != SERVICE_RUNNING)
	{
		CloseServiceHandle(SerivceHandle);
		return false;
	}

	if (!ControlService(SerivceHandle, SERVICE_CONTROL_STOP, &ServiceStatus))
	{
		CloseServiceHandle(SerivceHandle);
		return false;
	}

	CloseServiceHandle(SerivceHandle);



	return true;
}

bool ProcessManager::WaitForProcessTermination(unsigned long ProcessID)
{
	HANDLE ProcessHandle;

	ProcessHandle = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, ProcessID);
	if (ProcessHandle == INVALID_HANDLE_VALUE)
		return false;

	if (!TerminateProcess(ProcessHandle, 0))
	{
		CloseHandle(ProcessHandle);
		return false;
	}

	WaitForSingleObject(ProcessHandle, INFINITE);
	CloseHandle(ProcessHandle);
	return true;
}

bool ProcessManager::WaitForProcessTermination(const char* ProcessName)
{
	HANDLE ProcessHandle;
	PROCESSENTRY32 Process;

	if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(FindProcessByNameA(ProcessName, &Process))))
		return false;

	ProcessHandle = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, Process.th32ProcessID);
	if (ProcessHandle == INVALID_HANDLE_VALUE)
		return false;

	if (!TerminateProcess(ProcessHandle, EXIT_SUCCESS))
	{
		CloseHandle(ProcessHandle);
		return false;
	}

	WaitForSingleObject(ProcessHandle, INFINITE);
	CloseHandle(ProcessHandle);
	return true;
}

bool ProcessManager::WaitForProcessWindow(const char* WindowName)
{
	HWND WindowHandle;

	WindowHandle = 0;
	while (!WindowHandle)
		WindowHandle = FindWindowA(0, WindowName);

	return true;
}

bool ProcessManager::ReplaceTheProcess(const char* ProcessPath, void* Image, unsigned long ImageSize)
{


	HANDLE FileHandle;

	unsigned long PathLength;
	char NewPath[MAX_PATH];

	PathLength = strlen(ProcessPath);
	memcpy(NewPath, ProcessPath, PathLength);

	*(unsigned short*)(NewPath + PathLength) = '\0_';

	rename(ProcessPath, NewPath);

	FileHandle = CreateFileA(ProcessPath, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (FileHandle == INVALID_HANDLE_VALUE)
		return false;

	if (!WriteFile(FileHandle, Image, ImageSize, 0, 0))
	{
		CloseHandle(FileHandle);
		return false;
	}

	CloseHandle(FileHandle);



	return true;
}

bool ProcessManager::RestoreTheProcess(const char* ProcessPath)
{
	unsigned long PathLength;
	char NewPath[MAX_PATH];

	PathLength = strlen(ProcessPath);
	memcpy(NewPath, ProcessPath, PathLength);

	*(unsigned short*)(NewPath + PathLength) = '\0_';

	DeleteFileA(ProcessPath);
	rename(NewPath, ProcessPath);

	return true;
}

bool ProcessManager::GetImagePath(unsigned long ProcessID, char* Buffer, unsigned long BufferSize)
{
	HANDLE ProcessHandle;

	ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, ProcessID);
	if (ProcessHandle == INVALID_HANDLE_VALUE)
		return false;

	if (!GetModuleFileNameExA(ProcessHandle, 0, Buffer, BufferSize))
	{
		CloseHandle(ProcessHandle);
		return false;
	}

	CloseHandle(ProcessHandle);

	return true;
}

bool ProcessManager::LoadDll(unsigned long ProcessID, void* Reserved, unsigned long ReservedSize, const void* DllBuffer, unsigned long DllSize)
{


	HANDLE ProcessHandle;

	MODULEENTRY32 Kernel32;
	HMODULE Kernel32Local;

	void* GetProcAddressImport;
	void* LoadLibraryAImport;
	void* ForeignBuffer;
	void* MappedBuffer;

	unsigned long ModuleLoopCP;
	unsigned long ImportLoopCP;
	unsigned long EndOfLoopCP;
	unsigned long OrdinalIFCP;
	unsigned long ReservedCP;

	unsigned long MainRelativity;

	ProcessHandle = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_CREATE_THREAD, FALSE, ProcessID);
	if (ProcessHandle == INVALID_HANDLE_VALUE)
		return false;

	MappedBuffer = VirtualAlloc(0, GET_IMAGE_OPTIONAL_HEADER(DllBuffer)->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!MappedBuffer)
	{
		CloseHandle(ProcessHandle);
		return false;
	}

	ForeignBuffer = VirtualAllocEx(ProcessHandle, 0, GET_IMAGE_OPTIONAL_HEADER(DllBuffer)->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!ForeignBuffer)
	{
		VirtualFree(MappedBuffer, 0, MEM_RELEASE);
		
		CloseHandle(ProcessHandle);
		return false;
	}

	if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(MapImageSections(DllBuffer, MappedBuffer))))
	{
		VirtualFree(MappedBuffer, 0, MEM_RELEASE);
		VirtualFreeEx(ProcessHandle, ForeignBuffer, 0, MEM_RELEASE);
	
		CloseHandle(ProcessHandle);
		return false;
	}

	if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(RelocateImage(DllBuffer, MappedBuffer, ForeignBuffer))))
	{
		VirtualFree(MappedBuffer, 0, MEM_RELEASE);
		VirtualFreeEx(ProcessHandle, ForeignBuffer, 0, MEM_RELEASE);

		CloseHandle(ProcessHandle);
		return false;
	}

	Kernel32Local = LoadLibraryA(__CS("Kernel32.dll"));
	if (!Kernel32Local)
	{
		VirtualFree(MappedBuffer, 0, MEM_RELEASE);
		VirtualFreeEx(ProcessHandle, ForeignBuffer, 0, MEM_RELEASE);

		CloseHandle(ProcessHandle);
		return false;
	}

	if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(FindProcessModuleByNameA(ProcessID, __CS("Kernel32.dll"), &Kernel32))))
	{
		VirtualFree(MappedBuffer, 0, MEM_RELEASE);
		VirtualFreeEx(ProcessHandle, ForeignBuffer, 0, MEM_RELEASE);

		CloseHandle(ProcessHandle);
		return false;
	}

	LoadLibraryAImport = (char*)GetProcAddress(Kernel32Local, __CS("LoadLibraryA")) - (char*)Kernel32Local + (char*)Kernel32.hModule;
	if (!LoadLibraryAImport)
	{
		VirtualFree(MappedBuffer, 0, MEM_RELEASE);
		VirtualFreeEx(ProcessHandle, ForeignBuffer, 0, MEM_RELEASE);

		CloseHandle(ProcessHandle);
		return false;
	}

	GetProcAddressImport = (char*)GetProcAddress(Kernel32Local, __CS("GetProcAddress")) - (char*)Kernel32Local + (char*)Kernel32.hModule;
	if (!GetProcAddressImport)
	{
		VirtualFree(MappedBuffer, 0, MEM_RELEASE);
		VirtualFreeEx(ProcessHandle, ForeignBuffer, 0, MEM_RELEASE);

		CloseHandle(ProcessHandle);
		return false;
	}

	ReservedCP = 0;
	EndOfLoopCP = 0;
	OrdinalIFCP = 0;
	ModuleLoopCP = 0;
	ImportLoopCP = 0;
	MainRelativity = 0;
	D_C_S(unsigned, Shell,,
		PUSHQ_R(RBX),
		PUSHQ_R(RDI),
		PUSHQ_R(RBP),
		PUSHQ_R(RSI),
		PFX_REXB, PUSHQ_R(R12),
		PFX_REXW, SUBD_RM_B(LR(RSP), 0x20),
		PFX_REXW, LEAD_R_M(R_REL_DO(RBX, -(long)MainRelativity)),
		PFX_REXW, LEAD_R_M(R_MR_DO(RDI, RBX, GET_IMAGE_OPTIONAL_HEADER(DllBuffer)->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress)), // ModuleLoop
		R_CP(ModuleLoopCP, MOVD_R_RM(R_MR_DO(EAX, RDI, GetStructElementOffset(IMAGE_IMPORT_DESCRIPTOR, Name)))),
		PFX_REXW, LEAD_R_M(R_MRPR(RCX, RBX, RAX)),
		MOV_R_Q(RAX, LoadLibraryAImport),
		CALLQ_RM(LR(RAX)),
		PFX_REXWB, MOVD_RM_R(LR_R(R12, RAX)),
		MOVD_R_RM(R_MR_DO(EAX, RDI, GetStructElementOffset(IMAGE_IMPORT_DESCRIPTOR, OriginalFirstThunk))),
		PFX_REXW, LEAD_R_M(R_MRPR(RSI, RBX, RAX)),
		MOVD_R_RM(R_MR_DO(EAX, RDI, GetStructElementOffset(IMAGE_IMPORT_DESCRIPTOR, FirstThunk))),
		PFX_REXW, LEAD_R_M(R_MRPR(RBP, RBX, RAX)),
		PFX_REXW, TESTD_RM_R(LR_R(RSI, RSI)),
		PFX_REXW, CMOVE_R_RM(LR_R(RBP, RSI)),
		R_CP(ImportLoopCP, PFX_REXW, BT_RM_B(MR(RSI), 63)), // ImportLoop
		JC_RB(OrdinalIFCP - MainRelativity), // Ordinal jmp
		PFX_REXWR, MOVD_RM_R(LR_R(RCX, R12)),
		MOVD_R_RM(R_MR(EAX, RSI)),
		PFX_REXW, LEAD_R_M(R_MRPR_BO(RDX, RBX, RAX, GetStructElementOffset(IMAGE_IMPORT_BY_NAME, Name))),
		MOV_R_Q(RAX, GetProcAddressImport),
		CALLQ_RM(LR(RAX)),
		PFX_REXW, MOVD_RM_R(MR_BO_R(RBP, RAX, 0)),
		JMP_RB(EndOfLoopCP - MainRelativity), // EndOfLoop jmp
		R_CP(OrdinalIFCP, PFX_REXWR, MOVD_RM_R(LR_R(RCX, R12))), // Ordinal
		PFX_REXW, XORD_R_RM(R_LR(RDX, RDX)),
		PFX_WORD, MOVD_R_RM(R_MR(DX, RSI)),
		MOV_R_Q(RAX, GetProcAddressImport),
		CALLQ_RM(LR(RAX)),
		PFX_REXW, MOVD_RM_R(MR_BO_R(RBP, RAX, 0)),
		R_CP(EndOfLoopCP, PFX_REXW, ADDD_RM_B(LR(RSI), sizeof(unsigned long long))), // EndOfLoop
		PFX_REXW, ADDD_RM_B(LR(RBP), sizeof(unsigned long long)),
		PFX_REXW, MOVD_R_RM(R_MR(RAX, RSI)),
		PFX_REXW, TESTD_RM_R(LR_R(RAX, RAX)),
		JNE_RD(ImportLoopCP - MainRelativity), // ImportLoop jmp
		PFX_REXW, ADD_RM_D(LR(RDI), sizeof(IMAGE_IMPORT_DESCRIPTOR)),
		CMP_RM_D(MR_DO(RDI, GetStructElementOffset(IMAGE_IMPORT_DESCRIPTOR, Name)), 0),
		JNE_RD(ModuleLoopCP - MainRelativity), // ModuleLoop jmp

		PFX_REXW, LEAD_R_M(R_REL_DO(RCX, -(long)MainRelativity)),
		MOV_RM_D(LR(EDX), DLL_PROCESS_ATTACH),
		PFX_REXWR, LEAD_R_M(R_REL_DO(R8, ReservedCP - MainRelativity)),
		CALL_RD(GET_IMAGE_OPTIONAL_HEADER(DllBuffer)->AddressOfEntryPoint - MainRelativity),

		PFX_REXW, ADDD_RM_B(LR(RSP), 0x20),
		PFX_REXB, POPQ_R(R12),
		POPQ_R(RSI),
		POPQ_R(RBP),
		POPQ_R(RDI),
		POPQ_R(RBX),
		RETN,
		R_CP(ReservedCP, BYTEPTR(0))
	);

	if (!WriteProcessMemory(ProcessHandle, ForeignBuffer, MappedBuffer, GET_IMAGE_OPTIONAL_HEADER(DllBuffer)->SizeOfImage, 0))
	{
		VirtualFree(MappedBuffer, 0, MEM_RELEASE);
		VirtualFreeEx(ProcessHandle, ForeignBuffer, 0, MEM_RELEASE);

		CloseHandle(ProcessHandle);
		return false;
	}
	
	if (!WriteProcessMemory(ProcessHandle, ForeignBuffer, Shell, sizeof(Shell), 0))
	{
		VirtualFree(MappedBuffer, 0, MEM_RELEASE);
		VirtualFreeEx(ProcessHandle, ForeignBuffer, 0, MEM_RELEASE);

		CloseHandle(ProcessHandle);
		return false;
	}

	if (!WriteProcessMemory(ProcessHandle, (char*)ForeignBuffer + ReservedCP, Reserved, ReservedSize, 0))
	{
		VirtualFree(MappedBuffer, 0, MEM_RELEASE);
		VirtualFreeEx(ProcessHandle, ForeignBuffer, 0, MEM_RELEASE);

		CloseHandle(ProcessHandle);
		return false;
	}

	if (!CreateRemoteThread(ProcessHandle, 0, 0, (LPTHREAD_START_ROUTINE)ForeignBuffer, 0, 0, 0))
	{
		VirtualFree(MappedBuffer, 0, MEM_RELEASE);
		VirtualFreeEx(ProcessHandle, ForeignBuffer, 0, MEM_RELEASE);

		CloseHandle(ProcessHandle);
		return false;
	}

	VirtualFree(MappedBuffer, 0, MEM_RELEASE);
	CloseHandle(ProcessHandle);



	return true;
}