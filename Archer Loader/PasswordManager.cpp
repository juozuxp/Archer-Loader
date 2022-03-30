#include "ClientManager.h"
#include "PasswordManager.h"
#include <Imports.h>
#include "sha256.h"
#include "GUIManager.h"
#include "ConfigManager.h"
#include "HardwareManager.h"
#include "RSA.h"

#ifndef _DEBUG
#include <ThemidaSDK.h>
#endif

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

bool PasswordManager::ProcessLogin()
{


	unsigned long long EncryptedSize;
	void* EncryptedBuffer;

	Pair<unsigned long, unsigned long> KeyPair;
	LoginResponse Response;
	LoginType LoginType;
	LoginPacket Login;
	ArcherType Type;
	RSA Encryption;

	Type = ArcherType_None;
	while (Type != ArcherType_Key)
	{
		Type = ClientManager::Receive(&KeyPair, sizeof(KeyPair), 0);
		if (Type == ArcherType_Disconnect)
			return false;
	}

	Type = ArcherType_None;
	LoginType = LoginType_None;
	Response.Success = false;
	while (!Response.Success)
	{
		if (LoginType != LoginType_ForgotPassword)
		{
			if (Type != ArcherType_None)
				LoginType = GUIManager::PromptLogin(Login.Username, Login.Password, Login.RegistrationKey);
			else
			{
				if (!ConfigManager::GetSessionID(Login.SessionID))
					LoginType = GUIManager::PromptLogin(Login.Username, Login.Password, Login.RegistrationKey);
			}

			if (LoginType == LoginType_ForgotPassword)
				continue;
		}
		else
		{
			ResetType ResetType;

			ResetType = GUIManager::PromptReset(Login.ResetPassword, Login.ResetRegistrationKey);
			if (ResetType == ResetType_Login)
			{
				LoginType = LoginType_None;
				continue;
			}
		}

		switch (LoginType)
		{
		case LoginType_None:
		{
			Login.Type = LoginPacketType_ID;
		} break;
		case LoginType_Login:
		{
			Login.Type = LoginPacketType_Password;
		} break;
		case LoginType_Register:
		{
			Login.Type = LoginPacketType_Register;
		} break;
		case LoginType_ForgotPassword:
		{
			Login.Type = LoginPacketType_ResetPassword;
		} break;
		}

		Encryption = RSA(KeyPair);
		Encryption.Encrypt(&Login, sizeof(Login), &EncryptedBuffer, &EncryptedSize);

		if (!ClientManager::Send(ArcherType_Setup, EncryptedBuffer, EncryptedSize))
			return false;

		free(EncryptedBuffer);
		
		Type = ArcherType_None;
		while (Type != ArcherType_Setup)
		{
			Type = ClientManager::Receive(&Response, sizeof(Response), 0);
			if (Type == ArcherType_Disconnect)
				return false;
		}
	}

	if (Login.Type != LoginPacketType_ID)
		ConfigManager::UpdateConfig(Response.SessionID);
	


	return true;
}