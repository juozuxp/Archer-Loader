#include "UserInterface.h"
#include "UserManager.h"
#include "ClientManager.h"

bool UserInterface::AdminMultiSelect(const UserCommander* Commander, ClientManager& ClientManager, bool* IsSteam)
{
	unsigned char Buffer[sizeof(AdminPacket)];
	ArcherType PacketType;

	while (true)
	{
		PacketType = ClientManager.Receive(Buffer, sizeof(Buffer), 0);
		switch (PacketType)
		{
		case ArcherType_Key:
		{
			if (!UserManager::RedeemLicenseKey(Commander, ClientManager, (const char*)Buffer))
				return false;
		} break;
		case ArcherType_Manage:
		{
			AdminPacket* Packet;

			Packet = (AdminPacket*)Buffer;
			switch (Packet->Action)
			{
			case AdminAction_SyncUsers:
			{
				if (!SyncUsers(Commander, ClientManager))
					return false;
			} break;
			case AdminAction_ResetPassword:
			{
				const UserCommander* User;

				User = UserManager::GetUserByRegistration(Packet->RegistrationKey);
				if (User)
					UserManager::SetUserPassword(User, Packet->NewPassword);
			} break;
			case AdminAction_NewLicense:
			{
				char Key[LICENSEKEYSIZE];

				UserManager::GenerateLicenseKey(Key, Packet->LicenseLength);

				if (!ClientManager.Send(ArcherType_Manage, Key, LICENSEKEYSIZE))
					return false;
			} break;
			case AdminAction_NewRegistration:
			{
				char Key[REGISRATIONKEYSIZE];

				UserManager::GenerateRegistrationKey(Key, UserRole_User);

				if (!ClientManager.Send(ArcherType_Manage, Key, REGISRATIONKEYSIZE))
					return false;
			} break;
			}
		} break;
		case ArcherType_Setup:
		{
			*IsSteam = Buffer[0];
			return true;
		}
		case ArcherType_Disconnect:
		{
			return false;
		} break;
		}
	}

	return true;
}

bool UserInterface::SupportMultiSelect(const UserCommander* Commander, class ClientManager& ClientManager, bool* IsSteam)
{
	unsigned char Buffer[sizeof(AdminPacket)];
	ArcherType PacketType;

	while (true)
	{
		PacketType = ClientManager.Receive(Buffer, sizeof(Buffer), 0);
		switch (PacketType)
		{
		case ArcherType_Key:
		{
			if (!UserManager::RedeemLicenseKey(Commander, ClientManager, (const char*)Buffer))
				return false;
		} break;
		case ArcherType_Manage:
		{
			AdminPacket* Packet;

			Packet = (AdminPacket*)Buffer;
			switch (Packet->Action)
			{
			case AdminAction_SyncUsers:
			{
				if (!SyncUsers(Commander, ClientManager))
					return false;
			} break;
			}
		} break;
		case ArcherType_Setup:
		{
			*IsSteam = Buffer[0];
			return true;
		}
		case ArcherType_Disconnect:
		{
			return false;
		} break;
		}
	}

	return true;
}

bool UserInterface::ResellerMultiSelect(const UserCommander* Commander, class ClientManager& ClientManager, bool* IsSteam)
{
	unsigned char Buffer[sizeof(AdminPacket)];
	ArcherType PacketType;

	while (true)
	{
		PacketType = ClientManager.Receive(Buffer, sizeof(Buffer), 0);
		switch (PacketType)
		{
		case ArcherType_Key:
		{
			if (!UserManager::RedeemLicenseKey(Commander, ClientManager, (const char*)Buffer))
				return false;
		} break;
		case ArcherType_Manage:
		{
			AdminPacket* Packet;

			Packet = (AdminPacket*)Buffer;
			switch (Packet->Action)
			{
			case AdminAction_SyncUsers:
			{
				if (!SyncUsers(Commander, ClientManager))
					return false;
			} break;
			case AdminAction_NewLicense:
			{
				char Key[LICENSEKEYSIZE];

				UserManager::GenerateLicenseKey(Key, Packet->LicenseLength);

				if (!ClientManager.Send(ArcherType_Manage, Key, LICENSEKEYSIZE))
					return false;
			} break;
			case AdminAction_NewRegistration:
			{
				char Key[REGISRATIONKEYSIZE];

				UserManager::GenerateRegistrationKey(Key, UserRole_User);

				if (!ClientManager.Send(ArcherType_Manage, Key, REGISRATIONKEYSIZE))
					return false;
			} break;
			}
		} break;
		case ArcherType_Setup:
		{
			*IsSteam = Buffer[0];
			return true;
		}
		case ArcherType_Disconnect:
		{
			return false;
		} break;
		}
	}

	return true;
}

bool UserInterface::UserMultiSelect(const UserCommander* Commander, class ClientManager& ClientManager, bool* IsSteam)
{
	unsigned char Buffer[sizeof(AdminPacket)];
	ArcherType PacketType;

	while (true)
	{
		PacketType = ClientManager.Receive(Buffer, sizeof(Buffer), 0);
		switch (PacketType)
		{
		case ArcherType_Key:
		{
			if (!UserManager::RedeemLicenseKey(Commander, ClientManager, (const char*)Buffer))
				return false;
		} break;
		case ArcherType_Setup:
		{
			if (!Commander->IsSubscribed())
				break;

			*IsSteam = Buffer[0];
			return true;
		}
		case ArcherType_Disconnect:
		{
			return false;
		} break;
		}
	}

	return true;
}

bool UserInterface::SyncUsers(const UserCommander* Commander, class ClientManager& ClientManager)
{
	unsigned long long UserCount;

	UserInfo* RunUsers;
	UserInfo* Users;

	HashMap<char, UserCommander*>::Iterator Iterator = UserManager::IterateUsers();

	Users = (UserInfo*)malloc(UserManager::GetUserCount() * sizeof(UserInfo));
	RunUsers = Users;

	UserCount = 0;
	for (UserCommander* Current = Iterator; *Iterator; Current = Iterator++)
	{
		if (Current != Commander && Current->GetRole() >= Commander->GetRole())
			continue;

		RunUsers->SubscriptionTime = Current->GetSubscriptionTime();
		memcpy(RunUsers->Username, Current->GetUsername(), USERNAMESIZE);
		memcpy(RunUsers->Registration, Current->GetRegistrationKey(), REGISRATIONKEYSIZE);

		UserCount++;
		RunUsers++;
	}

	if (!ClientManager.Send(ArcherType_Manage, Users, UserCount * sizeof(UserInfo)))
		return false;

	UserManager::FinishUserIteration();

	free(Users);

	return true;
}