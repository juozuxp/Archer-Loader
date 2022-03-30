#include <WindowsEntrys.h>
#include "ClientManager.h"
#include "HookManager.h"
#include "ExceptionHandler.h"
#include "CryptString.h"
#include "DynamicImage.h"
#include "ExtensionManager.h"
#include <stdio.h>

#ifndef _DEBUG
#include <VirtualizerSDK.h>
#endif

void Thread(WSAPROTOCOL_INFOA* ProtocolInfo)
{


	DynamicImage Image;
	SOCKET ClientSocket;

	char Buffer[0x1000];
	unsigned long Error;

	ClientSocket = WSASocketA(0, 0, 0, ProtocolInfo, 0, WSA_FLAG_OVERLAPPED);
	if (!ClientSocket || ClientSocket == INVALID_SOCKET)
	{
		sprintf(Buffer, __CS("Cannot load error: %X"), WSAGetLastError() ^ 0x84562742);
		MessageBoxA(0, Buffer, __CS("Error"), MB_ICONERROR);
		exit(0);
	}

	ClientManager::Initialize(ClientSocket);

	if (!ClientManager::Send(ArcherType_Setup, 0, 0))
		exit(0);

	Image = DynamicImage(true);
	if (!Image.GetImageSize())
		exit(0);

	ExtensionManager::ExtendTheProcess(Image.GetImage(), Image.GetImageSize(), 0);

	HookManager::Instance.Initialize();
	ExceptionHandler::Instance.Initialize();


}

DLL_ENTRY(Instance, Reason, Reserved)
{


	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
	{
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Thread, Reserved, 0, 0);
	} break;
	}



	return TRUE;
}