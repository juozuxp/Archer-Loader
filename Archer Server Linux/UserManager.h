#pragma once
#include "HashMap.h"
#include "Mutex.h"
#include "Pair.h"

#define SESSIONIDSIZE 32
#define PASSWORDHASHSIZE 40

#define REGISRATIONKEYSIZE 33
#define LICENSEKEYSIZE 33

#define USERNAMESIZE 255
#define PASSWORDSIZE 255

enum UserRole : unsigned char
{
	UserRole_User,
	UserRole_Reseller,
	UserRole_Support,
	UserRole_Admin
};

class UserCommander
{
public:
	constexpr UserCommander()
	{
	}

	UserCommander(const char* Username, const char* Password, const char* RegistrationKey, const unsigned char* SessionID, UserRole Role);

public:
	constexpr const char* GetUsername() const
	{
		return Username;
	}

	constexpr const char* GetRegistrationKey() const
	{
		return RegistrationKey;
	}

	constexpr const unsigned char* GetSessionID() const
	{
		return SessionID;
	}

	constexpr unsigned long long GetSubscriptionTime() const
	{
		return SubTimeStamp;
	}

	constexpr UserRole GetRole() const
	{
		return Role;
	}

public:
	bool IsSubscribed() const;
	bool MatchPassword(const char* Password) const;

	void SetUserRole(UserRole Role);
	void SetPassword(const char* Password);
	void ReassignSession(const unsigned char* SessionID);
	void AddSubscriptionLength(unsigned long long SubSpan);

private:
	static void CreatePasswordHash(const char* Password, unsigned char* Hash, unsigned long long Seed);
	static void SaltPassword(const char* Password, unsigned long long SaltKey, void* SlatedPassword, unsigned short* SaltSize);

private:
	char Username[USERNAMESIZE] = { 0 };
	char RegistrationKey[REGISRATIONKEYSIZE] = { 0 };

	unsigned char SessionID[SESSIONIDSIZE] = { 0 };
	unsigned char Password[PASSWORDHASHSIZE] = { 0 };

	unsigned long long HardwareIDHash = 0;
	unsigned long long SubTimeStamp = 0;

	UserRole Role = UserRole_User;
};

class UserManager
{
public:
	static void Initialize();

	static unsigned long long GetUserCount();

	static void FinishUserIteration();
	static HashMap<char, UserCommander*>::Iterator IterateUsers();

	static const UserCommander* GetUserByRegistration(const char* Key);
	static const UserCommander* GetUserByUsername(const char* Username);

	static void SetUserRole(const UserCommander* Commander, UserRole Role);
	static void SetUserPassword(const UserCommander* Commander, const char* Password);

	static void GenerateRegistrationKey(char* Key, UserRole Role);
	static void GenerateLicenseKey(char* Key, unsigned long long SubLength);

	static bool RedeemLicenseKey(const UserCommander* Commander, class ClientManager& ClientManager, const char* Key);

	static const UserCommander* HandleLogin(class ClientManager& ClientManager);
	static bool ReportLicense(const UserCommander* Commander, class ClientManager& ClientManager);
	static bool HandleLicense(const UserCommander* Commander, class ClientManager& ClientManager);

private:
	void ScanUserDataBase();

	void GenerateSessionID(void* IDBuffer);

	bool InvalidateRegKey(const char* RegistrationKey, UserRole* Role);
	bool InvalidateLicenseKey(const char* LicenseKey, unsigned long long* SubLength);

	//void SafeReadUser(const char* Username, UserCommander* Commander);
	void SafeWriteUser(const UserCommander* Commander);

	UserCommander* Login(const unsigned char* SessionID);
	UserCommander* Login(const char* Username, const char* Password, unsigned char* SessionID);
	UserCommander* ResetUser(const char* RegistrationKey, const char* NewPassword, unsigned char* SessionID);
	UserCommander* CreateUser(const char* Username, const char* Password, const char* RegistrationKey, unsigned char* SessionID);

private:
	static bool IsValidUsername(const char* Username);

private:
	static UserManager Instance;

private:
	class DataBase* UserData = 0;
	class DataBase* LicenseData = 0;
	class DataBase* RegistrationData = 0;

	Mutex DataLock = Mutex();
	HashMap<char, UserCommander*> Users = HashMap<char, UserCommander*>(0);
	HashMap<char, UserCommander*> Registrations = HashMap<char, UserCommander*>(0);
	HashMap<unsigned char, UserCommander*> Sessions = HashMap<unsigned char, UserCommander*>(0);
};

