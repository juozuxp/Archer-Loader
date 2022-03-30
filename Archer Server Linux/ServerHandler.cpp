#include "ServerHandler.h"
#include "DynamicImage.h"
#include "CrashDumpHandler.h"
#include "CommandManager.h"
#include "UserManager.h"
#include "UserInterface.h"

Mutex ServerHandler::HandlerLocker = Mutex();

DynArray<ServerHandler*> ServerHandler::ActiveHandlers = DynArray<ServerHandler*>(0);
HashMap<const UserCommander*, ServerHandler*> ServerHandler::LoggedinHandlers = HashMap<const UserCommander*, ServerHandler*>(0);

ServerHandler::ServerHandler(bool)
{
	this->RegisterHandler();
	ManagedClient = ClientManager(false);
}

void ServerHandler::FinishHandlerIteration()
{
	HandlerLocker.Free();
}

DynArray<ServerHandler*>::Iterator ServerHandler::IterateHandlers()
{
	HandlerLocker.WaitForLock();
	return ActiveHandlers.GetIterator();
}

void ServerHandler::Terminate()
{
	ManagedClient.Disconnect();
}

void ServerHandler::Terminate(const char* HandlerIP)
{
	char IPAddress[MAX_IP_LENGTH];

	HandlerLocker.WaitForLock();

	DynArray<ServerHandler*>::Iterator Iterator = ActiveHandlers.GetIterator();
	for (ServerHandler* Current = Iterator; *Iterator; Current = Iterator++)
	{
		Current->GetSessionIPAddress(IPAddress);
		if (!strcmp(IPAddress, HandlerIP))
		{
			Current->Terminate();
			break;
		}
	}

	HandlerLocker.Free();
}

void ServerHandler::Terminate(UserCommander* Commander)
{
	ServerHandler* Handler;

	HandlerLocker.WaitForLock();

	Handler = LoggedinHandlers[Commander];
	if (Handler)
		Handler->Terminate();

	HandlerLocker.Free();
}

void ServerHandler::GetSessionIPAddress(char* IPAddress)
{
	ManagedClient.GetIPAddress(IPAddress);
}

void ServerHandler::ShutDown()
{
	UnregisterHandler();
	Terminate();
}

void ServerHandler::LoginHandler()
{
	unsigned long long Index;
	bool Reserved;

	HandlerLocker.WaitForLock();

	Index = LoggedinHandlers.ReserveIndex(LoggedInUser, &Reserved);
	if (!Reserved)
		LoggedinHandlers.GetByIndex(Index)->Terminate();

	LoggedinHandlers.GetByIndex(Index) = this;

	HandlerLocker.Free();
}

void ServerHandler::RegisterHandler()
{
	HandlerLocker.WaitForLock();

	ActiveHandlers.Add(this);

	HandlerLocker.Free();
}

void ServerHandler::UnregisterHandler()
{
	HandlerLocker.WaitForLock();

	ActiveHandlers.Remove(this);
	if (LoggedInUser)
	{
		unsigned long long Index;

		Index = LoggedinHandlers.GetIndex(LoggedInUser);
		if (Index != ~0 && LoggedinHandlers.GetByIndex(Index) == this)
			LoggedinHandlers.RemoveByIndex(Index);
	}

	HandlerLocker.Free();
}

void ServerHandler::CreateHandler()
{
	ServerHandler Handler = ServerHandler(false);

	Handler.SpinPackets();

	Handler.ShutDown();
}

bool ServerHandler::StreamImageFromDisk(const char* Path)
{
	DynamicImage Image;
	FILE* FileHandle;

	unsigned int FileSize;
	void* FileBuffer;

	FileHandle = fopen(Path, "r");
	if (!FileHandle)
		return false;

	fseek(FileHandle, 0L, SEEK_END);
	FileSize = ftell(FileHandle);
	rewind(FileHandle);
	if (!FileSize)
		return false;

	FileBuffer = malloc(FileSize);
	if (!FileBuffer)
		return false;

	if (!fread(FileBuffer, sizeof(unsigned char), FileSize, FileHandle))
		return false;

	fclose(FileHandle);

	Image = DynamicImage(FileBuffer, FileSize);
	if (!Image.StreamImage(ManagedClient))
		return false;

	free(FileBuffer);
	return true;
}

void ServerHandler::SpinPackets()
{
	unsigned char Buffer[0x1000];
	unsigned int ReceiveSize;
	bool ReservedIndex;

	ArcherType Type;

	LoggedInUser = UserManager::HandleLogin(ManagedClient);
	if (!LoggedInUser)
		return;

	LoginHandler();

	switch (LoggedInUser->GetRole())
	{
	case UserRole_User:
	{
		if (!StreamImageFromDisk("Archer Client.dll"))
			return;

		Type = ArcherType_None;
		while (Type != ArcherType_Setup)
		{
			Type = ManagedClient.Receive(0, 0, 0);
			if (Type == ArcherType_Disconnect)
				return;
		}

		if (!UserManager::ReportLicense(LoggedInUser, ManagedClient))
			return;

		if (!UserInterface::UserMultiSelect(LoggedInUser, ManagedClient, (bool*)&Buffer[0]))
			return;
	} break;
	case UserRole_Reseller:
	{
		if (!StreamImageFromDisk("Archer Client Reseller.dll"))
			return;

		Type = ArcherType_None;
		while (Type != ArcherType_Setup)
		{
			Type = ManagedClient.Receive(0, 0, 0);
			if (Type == ArcherType_Disconnect)
				return;
		}

		if (!UserManager::ReportLicense(LoggedInUser, ManagedClient))
			return;

		if (!UserInterface::SupportMultiSelect(LoggedInUser, ManagedClient, (bool*)&Buffer[0]))
			return;
	} break;
	case UserRole_Support:
	{
		if (!StreamImageFromDisk("Archer Client Support.dll"))
			return;

		Type = ArcherType_None;
		while (Type != ArcherType_Setup)
		{
			Type = ManagedClient.Receive(0, 0, 0);
			if (Type == ArcherType_Disconnect)
				return;
		}

		if (!UserManager::ReportLicense(LoggedInUser, ManagedClient))
			return;

		if (!UserInterface::SupportMultiSelect(LoggedInUser, ManagedClient, (bool*)&Buffer[0]))
			return;
	} break;
	case UserRole_Admin:
	{
		if (!StreamImageFromDisk("Archer Client Admin.dll"))
			return;

		Type = ArcherType_None;
		while (Type != ArcherType_Setup)
		{
			Type = ManagedClient.Receive(0, 0, 0);
			if (Type == ArcherType_Disconnect)
				return;
		}

		if (!UserManager::ReportLicense(LoggedInUser, ManagedClient))
			return;

		if (!UserInterface::AdminMultiSelect(LoggedInUser, ManagedClient, (bool*)&Buffer[0]))
			return;
	} break;
	}

	if (Buffer[0])
	{
		if (!StreamImageFromDisk("Archer CreateProcess Steam.exe"))
			return;
	}
	else
	{
		if (!StreamImageFromDisk("Archer CreateProcess.exe"))
			return;
	}

	Type = ArcherType_None;
	while (Type != ArcherType_Setup)
	{
		Type = ManagedClient.Receive(0, 0, 0);
		if (Type == ArcherType_Disconnect)
			return;
	}

	if (!StreamImageFromDisk("Archer Battleye.dll"))
		return;

	Type = ArcherType_None;
	while (Type != ArcherType_Setup)
	{
		Type = ManagedClient.Receive(0, 0, 0);
		if (Type == ArcherType_Disconnect)
			return;
	}

	if (!StreamImageFromDisk("ARK Internal.dll"))
		return;

	EmuClient.InitSession();
	while (true)
	{
		if (!LoggedInUser->IsSubscribed())
			break;

		Type = ManagedClient.Receive(Buffer, sizeof(Buffer), &ReceiveSize);

		if (Type == ArcherType_CrashDump)
		{
			memcpy(Buffer, LoggedInUser->GetUsername(), USERNAMESIZE);
			Buffer[USERNAMESIZE] = 0;

			CrashDumpHandler::Instance.CreateDump((const char*)Buffer, ManagedClient, *(unsigned int*)Buffer);
			break;
		}

		if (Type == ArcherType_Disconnect)
			break;

		if (Type == ArcherType_BatlleyeEnd)
			EmuClient.InitSession();

		if (Type == ArcherType_Battleye)
		{
			EmuClient.HandleResponse(ManagedClient, Buffer, ReceiveSize);
			if (!ManagedClient.Send(ArcherType_BatlleyeEnd, 0, 0))
				break;
		}
	}
}