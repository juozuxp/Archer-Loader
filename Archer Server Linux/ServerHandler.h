#pragma once
#include "ClientManager.h"
#include "BattleyeEmu.h"
#include "UserManager.h"
#include "DynArray.h"
#include "HashMap.h"
#include "Mutex.h"

class ServerHandler
{

private:
	ServerHandler(bool);

public:
	static void CreateHandler();

	static void Terminate(const char* HandlerIP);

	static void FinishHandlerIteration();
	static DynArray<ServerHandler*>::Iterator IterateHandlers();

public:
	void Terminate();
	void GetSessionIPAddress(char* IPAddress);

public:
	constexpr const UserCommander* GetLoggedInUser()
	{
		return LoggedInUser;
	}

	static constexpr unsigned int GetHandlerCount()
	{
		return ActiveHandlers.GetCount();
	}

private:
	void ShutDown();
	void SpinPackets();

	void LoginHandler();
	void RegisterHandler();
	void UnregisterHandler();

	bool StreamImageFromDisk(const char* Path);

private:
	static void Terminate(UserCommander* Commander);

private:
	BattleyeEmu EmuClient;
	ClientManager ManagedClient;
	const UserCommander* LoggedInUser = 0;

private:
	static Mutex HandlerLocker;

	static DynArray<ServerHandler*> ActiveHandlers;
	static HashMap<const UserCommander*, ServerHandler*> LoggedinHandlers;
};