#include "ServerHandler.h"
#include "CrashDumpHandler.h"
#include "CommandManager.h"
#include "DataBaseManager.h"
#include "UserManager.h"

int main()
{
	DataBaseManager::Initialize();
	UserManager::Initialize();

	CrashDumpHandler::Instance.Initialize();
	BattleyeEmu::Initialize();

	CommandManager::Initialize();
	ClientManager::StartListening();

	return 0;
}