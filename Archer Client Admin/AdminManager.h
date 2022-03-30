#pragma once

#define USERNAMESIZE 255
#define PASSWORDSIZE 255

#define SESSIONIDSIZE 32

#define LICENSEKEYSIZE 33
#define REGISRATIONKEYSIZE 33

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

class AdminManager
{
public:
	static bool ResetPassword(const char* RegistrationKey, const char* NewPassword);

	static bool CreateRegistration(char* Key);
	static bool CreateLicense(unsigned long long Length, char* Key);

	static bool SyncUsers(const UserInfo** Users, unsigned long long* UserCount);
};