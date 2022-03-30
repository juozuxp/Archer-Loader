#include "AdminManager.h"
#include "ClientManager.h"

bool AdminManager::ResetPassword(const char* RegistrationKey, const char* NewPassword)
{
	AdminPacket Packet;

	Packet.Action = AdminAction_ResetPassword;
	memcpy(Packet.NewPassword, NewPassword, PASSWORDSIZE);
	memcpy(Packet.RegistrationKey, RegistrationKey, REGISRATIONKEYSIZE);
	if (!ClientManager::Send(ArcherType_Manage, &Packet, sizeof(Packet)))
		return false;

	return true;
}

bool AdminManager::CreateRegistration(char* Key)
{
	AdminPacket Packet;
	ArcherType Type;

	*Key = 0;
	Packet.Action = AdminAction_NewRegistration;
	if (!ClientManager::Send(ArcherType_Manage, &Packet, sizeof(Packet)))
		return false;

	Type = ArcherType_None;
	while (Type != ArcherType_Manage)
	{
		Type = ClientManager::Receive(Key, REGISRATIONKEYSIZE);
		if (Type == ArcherType_Disconnect)
			return false;
	}

	return true;
}

bool AdminManager::CreateLicense(unsigned long long Length, char* Key)
{
	AdminPacket Packet;
	ArcherType Type;

	*Key = 0;
	Packet.LicenseLength = Length;
	Packet.Action = AdminAction_NewLicense;
	if (!ClientManager::Send(ArcherType_Manage, &Packet, sizeof(Packet)))
		return false;

	Type = ArcherType_None;
	while (Type != ArcherType_Manage)
	{
		Type = ClientManager::Receive(Key, LICENSEKEYSIZE);
		if (Type == ArcherType_Disconnect)
			return false;
	}

	return true;
}

bool AdminManager::SyncUsers(const UserInfo** Users, unsigned long long* UserCount)
{
	unsigned long UserSize;

	AdminPacket Packet;
	ArcherType Type;

	if (*Users)
		free((void*)*Users);

	Packet.Action = AdminAction_SyncUsers;
	if (!ClientManager::Send(ArcherType_Manage, &Packet, sizeof(Packet)))
		return false;

	Type = ArcherType_None;
	while (Type != ArcherType_Manage)
	{
		Type = ClientManager::Receive(0, 0, &UserSize);
		if (Type == ArcherType_Disconnect)
			return false;
	}

	*Users = (const UserInfo*)malloc(UserSize);
	*UserCount = UserSize / sizeof(const UserInfo);

	Type = ClientManager::Receive((UserInfo*)*Users, UserSize);
	if (Type == ArcherType_Disconnect)
		return false;

	return true;
}