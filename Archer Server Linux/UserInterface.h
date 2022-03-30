#pragma once
#include "UserManager.h"

enum AdminAction : unsigned char
{
	AdminAction_None,
	AdminAction_Leave,
	AdminAction_ResetPassword,
	AdminAction_NewLicense,
	AdminAction_NewRegistration,
	AdminAction_SyncUsers
};

struct UserInfo
{
	char Username[USERNAMESIZE];
	char Registration[REGISRATIONKEYSIZE];
	unsigned long long SubscriptionTime;
};

struct AdminPacket
{
	AdminAction Action;
	union
	{
		struct
		{
			char RegistrationKey[REGISRATIONKEYSIZE];
			char NewPassword[PASSWORDSIZE];
		};
		unsigned long long LicenseLength;
	};
};

class UserInterface
{
public:
	static bool AdminMultiSelect(const UserCommander* Commander, class ClientManager& ClientManager, bool* IsSteam);
	static bool SupportMultiSelect(const UserCommander* Commander, class ClientManager& ClientManager, bool* IsSteam);
	static bool ResellerMultiSelect(const UserCommander* Commander, class ClientManager& ClientManager, bool* IsSteam);
	static bool UserMultiSelect(const UserCommander* Commander, class ClientManager& ClientManager, bool* IsSteam);

private:
	static bool SyncUsers(const UserCommander* Commander, class ClientManager& ClientManager);
};