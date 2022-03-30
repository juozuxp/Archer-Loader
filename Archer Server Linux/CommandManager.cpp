#include "ServerHandler.h"
#include "CommandManager.h"
#include <pthread.h>
#include "BasicUtilities.h"
#include "UserManager.h"

CommandManager CommandManager::Instance;

constexpr Pair<Pair<const char*, const char*>, void(*)()> CommandManager::CommandList[];

void CommandManager::Initialize()
{
	unsigned long Thread;

	// pthread_create(&Thread, 0, (void*(*)(void*))CommanderThread, 0);
}

void CommandManager::CommanderThread()
{
	Instance.Commander();
}

void CommandManager::ListSessionsHandler()
{
	char Username[USERNAMESIZE + 1] = "Not logged in";
	char IPAddress[MAX_IP_LENGTH] = { 0 };

	DynArray<ServerHandler*>::Iterator Iterator = ServerHandler::IterateHandlers();

	printf("Handler Count: %d\n\n", ServerHandler::GetHandlerCount());
	for (ServerHandler* Current = Iterator; *Iterator; Current = Iterator++)
	{
		if (Current->GetLoggedInUser())
			memcpy(Username, Current->GetLoggedInUser()->GetUsername(), USERNAMESIZE);

		Username[USERNAMESIZE] = 0;

		Current->GetSessionIPAddress(IPAddress);
		printf("Username: %s IP: %s\n", Current->GetLoggedInUser() ? Current->GetLoggedInUser()->GetUsername() : "Not logged in", IPAddress);
	}

	ServerHandler::FinishHandlerIteration();
}

void CommandManager::LicenseKeyHandler()
{
	unsigned long long TimeSpan;
	char* CommandToken;

	char Key[LICENSEKEYSIZE];

	CommandToken = Instance.GetCommandToken();
	if (!CommandToken)
	{
		printf("Invalid time span\n");
		return;
	}

	UserManager::GenerateLicenseKey(Key, atoll(CommandToken));
	printf("%.*s\n", LICENSEKEYSIZE, Key);
}

void CommandManager::HelpHandler()
{
	for (unsigned int i = 0; i < GetArraySize(CommandList); i++)
		printf("%s - %s\n", CommandList[i].Element1.Element1, CommandList[i].Element1.Element2);
}

void CommandManager::TerminateSessionHandler()
{
	unsigned int Index;
	char* CommandToken;

	CommandToken = Instance.GetCommandToken();
	if (!CommandToken)
	{
		printf("Invalid IP\n");
		return;
	}

	ServerHandler::Terminate(CommandToken);
}

void CommandManager::TerminateAllSessionsHandler()
{
	ArrayBase<ServerHandler*>::Iterator Iterator = ServerHandler::IterateHandlers();
	for (ServerHandler* Handler = Iterator; *Iterator; Handler = Iterator++)
		Handler->Terminate();

	ServerHandler::FinishHandlerIteration();
}

void CommandManager::ManageUserHandler()
{
	const UserCommander* User;

	unsigned int Index;
	char* CommandToken;

	CommandToken = Instance.GetCommandToken();
	if (!CommandToken)
	{
		printf("Invalid User\n");
		return;
	}

	User = UserManager::GetUserByUsername(CommandToken);
	if (!User)
	{
		printf("Invalid User\n");
		return;
	}
	
	while (true)
	{
		CommandToken = Instance.GetCommandToken();
		if (!CommandToken)
			return;
		else if (!strcmp(CommandToken, "-ps"))
		{
			CommandToken = Instance.GetCommandToken();
			if (!CommandToken)
			{
				printf("Invalid Password\n");
				return;
			}

			UserManager::SetUserPassword(User, CommandToken);
		}
		else if (!strcmp(CommandToken, "-rs"))
		{
			CommandToken = Instance.GetCommandToken();
			if (!CommandToken)
			{
				printf("Invalid Role\n");
				return;
			}

			if (!strcasecmp(CommandToken, "admin"))
				UserManager::SetUserRole(User, UserRole_Admin);
			else if (!strcasecmp(CommandToken, "user"))
				UserManager::SetUserRole(User, UserRole_User);
			else if (!strcasecmp(CommandToken, "support"))
				UserManager::SetUserRole(User, UserRole_Support);
			else if (!strcasecmp(CommandToken, "reseller"))
				UserManager::SetUserRole(User, UserRole_Reseller);
			else
			{
				printf("Invalid Role\n");
				return;
			}
		}
	}
}

void CommandManager::ListUserHandler()
{
	HashMap<char, UserCommander*>::Iterator Iterator = UserManager::IterateUsers();
	for (UserCommander* Current = Iterator; *Iterator; Current = Iterator++)
		printf("Username: %.*s RegistrationKey: %.*s Subscribed: %s\n", USERNAMESIZE, Current->GetUsername(), REGISRATIONKEYSIZE, Current->GetRegistrationKey(), Current->IsSubscribed() ? "yes" : "no");

	UserManager::FinishUserIteration();
}

void CommandManager::CommandLine(char* Line)
{
	unsigned int TokenLength;
	char* Token;

	Token = strtok(Line, " ");
	if (!Token || *Token == '\n')
		return;

	TokenLength = strlen(Token);
	if (*(Token + TokenLength - 1) == '\n')
		*(Token + TokenLength - 1) = '\0';

	for (unsigned int i = 0; i < GetArraySize(CommandList); i++)
	{
		if (!strcmp(Token, CommandList[i].Element1.Element1))
		{
			CommandList[i].Element2();
			return;
		}
	}

	printf("Invalid command\n");
}

char* CommandManager::GetCommandToken()
{
	unsigned int TokenLength;
	char* Token;

	Token = strtok(0, " ");
	if (!Token || *Token == '\n')
		return 0;

	TokenLength = strlen(Token);
	if (*(Token + TokenLength - 1) == '\n')
		*(Token + TokenLength - 1) = '\0';

	return Token;
}

void CommandManager::ClearConsoleHandler()
{
	system("clear");
}

void CommandManager::RegistrationKeyHandler()
{
	char Key[REGISRATIONKEYSIZE];

	UserRole Role;
	char* Token;

	Token = Instance.GetCommandToken();
	if (!Token)
		Role = UserRole_User;
	else
	{
		while (true)
		{
			if (!strcmp(Token, "-r"))
			{
				Token = Instance.GetCommandToken();
				if (!Token)
				{
					printf("Invalid Role\n");
					return;
				}

				if (!strcasecmp(Token, "admin"))
					Role = UserRole_Admin;
				else if (!strcasecmp(Token, "user"))
					Role = UserRole_User;
				else if (!strcasecmp(Token, "support"))
					Role = UserRole_Support;
				else if (!strcasecmp(Token, "reseller"))
					Role = UserRole_Reseller;
				else
				{
					printf("Invalid Role\n");
					return;
				}
			}

			Token = Instance.GetCommandToken();
			if (!Token)
				break;
		}
	}

	UserManager::GenerateRegistrationKey(Key, Role);
	printf("%.*s\n", REGISRATIONKEYSIZE, Key);
}

void CommandManager::Commander()
{
	char Buffer[0x1000];

	while (true)
	{
		printf("Command Line >> ");
		fgets(Buffer, sizeof(Buffer), stdin);

		CommandLine(Buffer);
	}
}