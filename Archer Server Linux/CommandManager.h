#pragma once
#include "Pair.h"

class CommandManager
{
public:
	static void Initialize();

private:
	static void CommanderThread();

private:
	void Commander();
	void CommandLine(char* Line);

	char* GetCommandToken();

private:
	static void HelpHandler();
	static void ListUserHandler();
	static void ManageUserHandler();
	static void LicenseKeyHandler();
	static void ListSessionsHandler();
	static void ClearConsoleHandler();
	static void RegistrationKeyHandler();
	static void TerminateSessionHandler();
	static void TerminateAllSessionsHandler();

private:
	static CommandManager Instance;

private:
	constexpr static Pair<Pair<const char*, const char*>, void(*)()> CommandList[] =
	{
		Pair<Pair<const char*, const char*>, void(*)()>(Pair<const char*, const char*>("help", "Lists all commands"), HelpHandler),
		Pair<Pair<const char*, const char*>, void(*)()>(Pair<const char*, const char*>("ls", "Lists all sessions"), ListSessionsHandler),
		Pair<Pair<const char*, const char*>, void(*)()>(Pair<const char*, const char*>("ts", "Terminate a session by index"), TerminateSessionHandler),
		Pair<Pair<const char*, const char*>, void(*)()>(Pair<const char*, const char*>("tas", "Terminate all sessions"), TerminateAllSessionsHandler),
		Pair<Pair<const char*, const char*>, void(*)()>(Pair<const char*, const char*>("clear", "Clears console"), ClearConsoleHandler),
		Pair<Pair<const char*, const char*>, void(*)()>(Pair<const char*, const char*>("newregister", "Generates registration key"), RegistrationKeyHandler),
		Pair<Pair<const char*, const char*>, void(*)()>(Pair<const char*, const char*>("newlicense", "Generates license key"), LicenseKeyHandler),
		Pair<Pair<const char*, const char*>, void(*)()>(Pair<const char*, const char*>("listuser", "Lists all users"), ListUserHandler),
		Pair<Pair<const char*, const char*>, void(*)()>(Pair<const char*, const char*>("manuser", "Manage user -ps - set password"), ManageUserHandler),
	};
};

