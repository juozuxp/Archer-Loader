#include "UserManager.h"
#include "Randomizer.h"
#include "PseudoRandom.h"
#include "SHA256.h"
#include "ClientManager.h"
#include "RSA.h"
#include "DataBaseManager.h"
#include <stdio.h>

UserManager UserManager::Instance;

enum LoginPacketType : unsigned char
{
	LoginPacketType_Password,
	LoginPacketType_Register,
	LoginPacketType_ID,
	LoginPacketType_ResetPassword
};

struct LoginPacket
{
	LoginPacketType Type;
	union
	{
		struct
		{
			char Username[USERNAMESIZE];
			char Password[PASSWORDSIZE];
			char RegistrationKey[REGISRATIONKEYSIZE];
		};

		struct
		{
			char ResetPassword[PASSWORDSIZE];
			char ResetRegistrationKey[REGISRATIONKEYSIZE];
		};

		unsigned char SessionID[SESSIONIDSIZE];
	};
};

struct LoginResponse
{
	bool Success;
	unsigned char SessionID[SESSIONIDSIZE];
};

UserCommander::UserCommander(const char* Username, const char* Password, const char* RegistrationKey, const unsigned char* SessionID, UserRole Role)
{
	this->Role = Role;
	this->HardwareIDHash = 0;

	CreatePasswordHash(Password, this->Password, 0);

	memcpy(this->Username, Username, strnlen(Username, USERNAMESIZE));
	memcpy(this->SessionID, SessionID, SESSIONIDSIZE);
	memcpy(this->RegistrationKey, RegistrationKey, REGISRATIONKEYSIZE);
}

void UserCommander::AddSubscriptionLength(unsigned long long SubSpan)
{
	if (IsSubscribed())
	{
		SubTimeStamp += SubSpan;
		return;
	}

	SubTimeStamp = time(0) + SubSpan;
}

bool UserCommander::IsSubscribed() const
{
	return SubTimeStamp > time(0);
}

void UserCommander::ReassignSession(const unsigned char* SessionID)
{
	memcpy(this->SessionID, SessionID, SESSIONIDSIZE);
}

bool UserCommander::MatchPassword(const char* Password) const
{
	unsigned char PasswordHash[PASSWORDHASHSIZE];

	CreatePasswordHash(Password, PasswordHash, *(unsigned long long*)this->Password);

	return !memcmp(this->Password, PasswordHash, PASSWORDHASHSIZE);
}

void UserCommander::SetPassword(const char* Password)
{
	CreatePasswordHash(Password, this->Password, 0);
}

void UserCommander::SetUserRole(UserRole Role)
{
	this->Role = Role;
}

void UserCommander::CreatePasswordHash(const char* Password, unsigned char* Hash, unsigned long long Seed)
{
	unsigned char Salted[PASSWORDSIZE * 2];
	unsigned long long CurrentSeed;
	unsigned short PasswordSize;

	if (!Seed)
		CurrentSeed = Randomizer::RandomNumber();
	else
		CurrentSeed = Seed;

	memcpy(Hash, &CurrentSeed, sizeof(CurrentSeed));
	SaltPassword(Password, CurrentSeed, Salted, &PasswordSize);
	SHA256::Hash(Salted, PasswordSize, Hash + sizeof(CurrentSeed));
}

void UserCommander::SaltPassword(const char* Password, unsigned long long SaltKey, void* SlatedPassword, unsigned short* SaltSize)
{
	PseudoRandom Randomizer = PseudoRandom(SaltKey);

	*SaltSize = (Randomizer.GetNumber() % PASSWORDSIZE) + PASSWORDSIZE;
	memset(SlatedPassword, 0, *SaltSize);
	for (unsigned long long i = 0; i < PASSWORDSIZE && *Password; i++, Password++)
		((unsigned char*)SlatedPassword)[Randomizer.GetNumber() % *SaltSize] ^= *Password;

	for (unsigned long long i = 0; i < *SaltSize; i++)
	{
		if (!((char*)SlatedPassword)[i])
			((char*)SlatedPassword)[i] = Randomizer.GetNumber();
	}
}

void UserManager::Initialize()
{
	Instance.UserData = DataBaseManager::OpenDataBase("Users");
	Instance.LicenseData = DataBaseManager::OpenDataBase("Licenses");
	Instance.RegistrationData = DataBaseManager::OpenDataBase("Registrations");

	Instance.ScanUserDataBase();
}

void UserManager::ScanUserDataBase()
{
	DataLock.WaitForLock();

	DataBase::Iterator<UserCommander> Iterator = UserData->GetEntryIterator<UserCommander>();
	for (const UserCommander* Current = &Iterator; *Iterator; Current = &Iterator++)
	{
		UserCommander* Commander;

		Commander = (UserCommander*)malloc(sizeof(UserCommander));
		if (!Commander)
			continue;

		memcpy(Commander, Current, sizeof(UserCommander));

		Users.Add(Commander->GetUsername(), USERNAMESIZE, Commander);
		Sessions.Add(Commander->GetSessionID(), SESSIONIDSIZE, Commander);
		Registrations.Add(Commander->GetRegistrationKey(), REGISRATIONKEYSIZE, Commander);
	}

	DataLock.Free();
}

const UserCommander* UserManager::HandleLogin(class ClientManager& ClientManager)
{
	unsigned char Packet[0x1000];

	unsigned int PacketSize;

	Pair<unsigned int, unsigned int> KeyPair;
	UserCommander* Commander;
	LoginPacket* LoginPacket;
	LoginResponse Response;
	ArcherType Type;
	RSA Encryption;

	KeyPair = Encryption.GenerateEncryptionKeys();
	if (!ClientManager.Send(ArcherType_Key, &KeyPair, sizeof(KeyPair)))
		return 0;

	Response.Success = false;
	while (!Response.Success)
	{
		Type = ArcherType_None;
		while (Type != ArcherType_Setup)
		{
			Type = ClientManager.Receive(Packet, sizeof(Packet), &PacketSize, 180);
			if (Type == ArcherType_Disconnect)
				return 0;
		}

		if (PacketSize == Encryption.CryptSize(sizeof(::LoginPacket)))
		{
			Encryption.Decrypt(Packet, PacketSize, (void**)&LoginPacket, 0);

			switch (LoginPacket->Type)
			{
			case LoginPacketType_Register:
			{
				if (*LoginPacket->Password && *LoginPacket->Username)
					Commander = Instance.CreateUser(LoginPacket->Username, LoginPacket->Password, LoginPacket->RegistrationKey, Response.SessionID);
			} break;
			case LoginPacketType_ID:
			{
				Commander = Instance.Login(LoginPacket->SessionID);
			} break;
			case LoginPacketType_Password:
			{
				if (*LoginPacket->Password && *LoginPacket->Username)
					Commander = Instance.Login(LoginPacket->Username, LoginPacket->Password, Response.SessionID);
			} break;
			case LoginPacketType_ResetPassword:
			{
				if (*LoginPacket->ResetPassword)
					Commander = Instance.ResetUser(LoginPacket->ResetRegistrationKey, LoginPacket->ResetPassword, Response.SessionID);
			} break;
			}

			free(LoginPacket);
		}

		Response.Success = Commander;

		if (!ClientManager.Send(ArcherType_Setup, &Response, sizeof(Response)))
			return 0;
	}

	return Commander;
}

bool UserManager::ReportLicense(const UserCommander* Commander, class ClientManager& ClientManager)
{
	unsigned long long TimeStamp;

	TimeStamp = Commander->GetSubscriptionTime();
	if (!ClientManager.Send(ArcherType_Key, &TimeStamp, sizeof(TimeStamp)))
		return false;

	return true;
}

unsigned long long UserManager::GetUserCount()
{
	return Instance.Users.GetCount();
}

bool UserManager::RedeemLicenseKey(const UserCommander* Commander, ClientManager& ClientManager, const char* Key)
{
	unsigned long long TimeStamp;
	unsigned long long TimeSpan;

	TimeStamp = Commander->GetSubscriptionTime();
	if (Instance.InvalidateLicenseKey(Key, &TimeSpan))
	{
		((UserCommander*)Commander)->AddSubscriptionLength(TimeSpan);
		TimeStamp = Commander->GetSubscriptionTime();
	}

	if (!ClientManager.Send(ArcherType_Key, &TimeStamp, sizeof(TimeStamp)))
		return false;

	Instance.SafeWriteUser(Commander);

	return true;
}

bool UserManager::HandleLicense(const UserCommander* Commander, class ClientManager& ClientManager)
{
	unsigned long long TimeStamp;
	unsigned long long TimeSpan;

	char Key[LICENSEKEYSIZE];

	ArcherType Type;

	TimeStamp = Commander->GetSubscriptionTime();

	if (!ClientManager.Send(ArcherType_Key, &TimeStamp, sizeof(TimeStamp)))
		return false;

	if (Commander->IsSubscribed())
		return true;

	TimeSpan = 0;
	while (!Commander->IsSubscribed())
	{
		Type = ArcherType_None;
		while (Type != ArcherType_Key)
		{
			Type = ClientManager.Receive(Key, sizeof(Key), 0);
			if (Type == ArcherType_Disconnect)
				return false;
		}

		if (!RedeemLicenseKey(Commander, ClientManager, Key))
			return false;
	}

	return true;
}

void UserManager::GenerateRegistrationKey(char* Key, UserRole Role)
{
	char KeyCopy[REGISRATIONKEYSIZE + 1];
	char* RunKey;

	UserRole RoleCopy;

	Instance.DataLock.WaitForLock();

	KeyCopy[0] = 'R';
	KeyCopy[REGISRATIONKEYSIZE] = 0;
	do
	{
		RunKey = KeyCopy + 1;
		for (unsigned char i = 0; i < (REGISRATIONKEYSIZE - 1) / (sizeof(unsigned long long) * 2); i++, RunKey += sizeof(unsigned long long) * 2)
			sprintf(RunKey, "%016llX", Randomizer::RandomNumber());
	} while (Instance.Registrations.Contains(KeyCopy, REGISRATIONKEYSIZE));

	Instance.DataLock.Free();

	RoleCopy = Role;
	Instance.RegistrationData->WriteEntry<UserRole>(KeyCopy, RoleCopy);

	memcpy(Key, KeyCopy, REGISRATIONKEYSIZE);
}

void UserManager::GenerateLicenseKey(char* Key, unsigned long long SubLength)
{
	char KeyCopy[LICENSEKEYSIZE + 1];
	unsigned long long Length;
	char* RunKey;

	KeyCopy[0] = 'L';
	KeyCopy[LICENSEKEYSIZE] = 0;
	do
	{
		RunKey = KeyCopy + 1;
		for (unsigned char i = 0; i < (LICENSEKEYSIZE - 1) / (sizeof(unsigned long long) * 2); i++, RunKey += sizeof(unsigned long long) * 2)
			sprintf(RunKey, "%016llX", Randomizer::RandomNumber());
	} while (Instance.LicenseData->ContainEntry(KeyCopy));

	Length = SubLength;
	Instance.LicenseData->WriteEntry<unsigned long long>(KeyCopy, Length);

	memcpy(Key, KeyCopy, LICENSEKEYSIZE);
}

UserCommander* UserManager::Login(const unsigned char* SessionID)
{
	unsigned long long SessionIndex;

	UserCommander* Commander;

	DataLock.WaitForLock();

	SessionIndex = Sessions.GetIndex(SessionID, SESSIONIDSIZE);
	if (SessionIndex == ~0ull)
	{
		DataLock.Free();
		return 0;
	}

	Commander = Sessions.GetByIndex(SessionIndex);

	DataLock.Free();

	if (!Commander)
		return 0;

	return Commander;
}

UserCommander* UserManager::Login(const char* Username, const char* Password, unsigned char* SessionID)
{
	unsigned long long UserIndex;

	UserCommander* Commander;

	DataLock.WaitForLock();
	
	UserIndex = Users.GetIndex(Username, USERNAMESIZE);
	if (UserIndex == ~0ull)
	{
		DataLock.Free();
		return 0;
	}

	Commander = Users.GetByIndex(UserIndex);

	DataLock.Free();

	if (!Commander)
		return 0;

	if (!Commander->MatchPassword(Password))
		return 0;

	GenerateSessionID(SessionID);
	Commander->ReassignSession(SessionID);

	DataLock.WaitForLock();

	Sessions.Add(SessionID, SESSIONIDSIZE, Commander);

	DataLock.Free();

	SafeWriteUser(Commander);
	return Commander;
}

UserCommander* UserManager::ResetUser(const char* RegistrationKey, const char* NewPassword, unsigned char* SessionID)
{
	unsigned long long UserIndex;

	UserCommander* Commander;

	DataLock.WaitForLock();

	UserIndex = Registrations.GetIndex(RegistrationKey, REGISRATIONKEYSIZE);
	if (UserIndex == ~0ull)
	{
		DataLock.Free();
		return 0;
	}

	Commander = Registrations.GetByIndex(UserIndex);

	DataLock.Free();

	if (!Commander)
		return 0;

	Commander->SetPassword(NewPassword);

	GenerateSessionID(SessionID);
	Commander->ReassignSession(SessionID);

	DataLock.WaitForLock();

	Sessions.Add(SessionID, SESSIONIDSIZE, Commander);

	DataLock.Free();

	SafeWriteUser(Commander);
	return Commander;

}

bool UserManager::InvalidateRegKey(const char* RegistrationKey, UserRole* Role)
{
	char KeyBuffer[REGISRATIONKEYSIZE + 1];

	KeyBuffer[REGISRATIONKEYSIZE] = 0;
	memcpy(KeyBuffer, RegistrationKey, REGISRATIONKEYSIZE);
	if (!RegistrationData->ReadEntry<UserRole>(KeyBuffer, Role))
		return false;

	return RegistrationData->DeleteEntry(KeyBuffer);
}

bool UserManager::InvalidateLicenseKey(const char* LicenseKey, unsigned long long* SubLength)
{
	char KeyBuffer[LICENSEKEYSIZE + 1];

	KeyBuffer[LICENSEKEYSIZE] = 0;
	memcpy(KeyBuffer, LicenseKey, LICENSEKEYSIZE);
	if (!LicenseData->ReadEntry<unsigned long long>(KeyBuffer, SubLength))
		return false;

	return LicenseData->DeleteEntry(KeyBuffer);
}

bool UserManager::IsValidUsername(const char* Username)
{
	const char* RunName;
	bool ValidChar;

	ValidChar = false;
	RunName = Username;
	for (unsigned short i = 0; i < USERNAMESIZE && *RunName; i++, RunName++)
	{
		if (*RunName == '/')
			return false;

		if (*RunName != '.')
			ValidChar = true;
	}

	return ValidChar;
}

void UserManager::GenerateSessionID(void* IDBuffer)
{
	void* RunIDBuffer;

	DataLock.WaitForLock();

	do
	{
		RunIDBuffer = IDBuffer;
		for (unsigned char i = 0; i < SESSIONIDSIZE / sizeof(unsigned long long); i++, RunIDBuffer = ((unsigned long long*)RunIDBuffer) + 1)
			*((unsigned long long*)RunIDBuffer) = Randomizer::RandomNumber();
	} while (Sessions.Contains((unsigned char*)IDBuffer, SESSIONIDSIZE));

	Sessions.Add((unsigned char*)IDBuffer, SESSIONIDSIZE, 0);

	DataLock.Free();
}

UserCommander* UserManager::CreateUser(const char* Username, const char* Password, const char* RegistrationKey, unsigned char* SessionID)
{
	UserCommander* Commander;
	UserRole Role;

	if (!IsValidUsername(Username))
		return 0;

	DataLock.WaitForLock();
	if (Users.Contains(Username, USERNAMESIZE))
	{
		DataLock.Free();
		return 0;
	}

	DataLock.Free();

	if (!InvalidateRegKey(RegistrationKey, &Role))
		return 0;

	Commander = (UserCommander*)malloc(sizeof(UserCommander));
	if (!Commander)
		return 0;

	GenerateSessionID(SessionID);
	*Commander = UserCommander(Username, Password, RegistrationKey, SessionID, Role);

	DataLock.WaitForLock();

	Users.Add(Username, USERNAMESIZE, Commander);
	Sessions.Add(SessionID, SESSIONIDSIZE, Commander);
	Registrations.Add(Commander->GetRegistrationKey(), REGISRATIONKEYSIZE, Commander);

	DataLock.Free();

	SafeWriteUser(Commander);
	return Commander;
}

void UserManager::SetUserRole(const UserCommander* Commander, UserRole Role)
{
	((UserCommander*)Commander)->SetUserRole(Role);
	Instance.SafeWriteUser(Commander);
}

void UserManager::SetUserPassword(const UserCommander* Commander, const char* Password)
{
	char Buffer[PASSWORDSIZE + 1];

	memset(Buffer, 0, PASSWORDSIZE);
	strcpy(Buffer, Password);

	((UserCommander*)Commander)->SetPassword(Buffer);
	Instance.SafeWriteUser(Commander);
}

void UserManager::SafeWriteUser(const UserCommander* Commander)
{
	char Buffer[USERNAMESIZE + 1];

	Buffer[USERNAMESIZE] = 0;
	memcpy(Buffer, Commander->GetUsername(), USERNAMESIZE);
	UserData->WriteEntry<UserCommander>(Buffer, *Commander);
}

void UserManager::FinishUserIteration()
{
	Instance.DataLock.Free();
}

HashMap<char, UserCommander*>::Iterator UserManager::IterateUsers()
{
	Instance.DataLock.WaitForLock();
	return Instance.Users.GetIterator();
}

const UserCommander* UserManager::GetUserByUsername(const char* Username)
{
	char Buffer[USERNAMESIZE];

	UserCommander* Commander;

	Instance.DataLock.WaitForLock();

	memset(Buffer, 0, USERNAMESIZE);
	strcpy(Buffer, Username);
	Commander = Instance.Users.Get(Buffer, USERNAMESIZE);

	Instance.DataLock.Free();

	return Commander;
}

const UserCommander* UserManager::GetUserByRegistration(const char* Key)
{
	UserCommander* Commander;

	Instance.DataLock.WaitForLock();

	Commander = Instance.Registrations.Get(Key, REGISRATIONKEYSIZE);

	Instance.DataLock.Free();

	return Commander;
}