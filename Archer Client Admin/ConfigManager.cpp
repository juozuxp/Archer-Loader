#include "ConfigManager.h"
#include <BasicUtilities.h>
#include <shlobj.h>
#include <intrin.h>
#include "CryptString.h"
#include "Randomizer.h"

#ifndef _DEBUG
#include <VirtualizerSDK.h>
#endif

ConfigManager ConfigManager::Instance;

void ConfigManager::EncryptConfig()
{
	unsigned char EncStartIndex;
	unsigned long PacketProgress;
	unsigned long long* RunPacket;
	const unsigned long long* LocalEncryption;

	LoadedConfig.Key = Randomizer::RandomNumber();
	EncStartIndex = ((LoadedConfig.Key >> 25) ^ (LoadedConfig.Key >> 9) ^ (LoadedConfig.Key >> 42) ^ (LoadedConfig.Key >> 14)) % GetArraySize(Keys);

	PacketProgress = sizeof(GameConfig) - GetStructElementOffset(GameConfig, Buffer);

	RunPacket = (unsigned long long*)LoadedConfig.Buffer;
	LocalEncryption = &Keys[EncStartIndex];
	for (unsigned char i = EncStartIndex; (PacketProgress & ~(sizeof(unsigned long long) - 1)); i++, RunPacket++, LocalEncryption++, PacketProgress -= sizeof(unsigned long long))
	{
		unsigned long long Hash;

		if (i >= GetArraySize(Keys))
		{
			i = 0;
			LocalEncryption = Keys;
		}

		Hash = *LocalEncryption ^ LoadedConfig.Key;

		if (Hash)
			*RunPacket ^= Hash;
		else
			*RunPacket ^= *LocalEncryption;
	}

	if (PacketProgress >= sizeof(unsigned long))
	{
		unsigned long Hash;

		Hash = Keys[5] ^ (LoadedConfig.Key & ((1 << 32) - 1));
		if (Hash)
			(*(unsigned long*)RunPacket) ^= Hash;
		else
			(*(unsigned long*)RunPacket) ^= Keys[0];

		PacketProgress -= sizeof(unsigned long);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned long));
	}

	if (PacketProgress >= sizeof(unsigned short))
	{
		unsigned short Hash;

		Hash = Keys[8] ^ (LoadedConfig.Key & ((1 << 16) - 1));
		if (Hash)
			(*(unsigned short*)RunPacket) ^= Hash;
		else
			(*(unsigned short*)RunPacket) ^= Keys[3];

		PacketProgress -= sizeof(unsigned short);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned short));
	}

	if (PacketProgress >= sizeof(unsigned char))
	{
		unsigned char Hash;

		Hash = Keys[4] ^ (LoadedConfig.Key & ((1 << 8) - 1));
		if (Hash)
			(*(unsigned char*)RunPacket) ^= Hash;
		else
			(*(unsigned char*)RunPacket) ^= Keys[5];

		PacketProgress -= sizeof(unsigned char);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned char));
	}
}

void ConfigManager::DecryptConfig()
{
	unsigned char EncStartIndex;
	unsigned long PacketProgress;
	unsigned long long* RunPacket;
	const unsigned long long* LocalEncryption;

	EncStartIndex = ((LoadedConfig.Key >> 25) ^ (LoadedConfig.Key >> 9) ^ (LoadedConfig.Key >> 42) ^ (LoadedConfig.Key >> 14)) % GetArraySize(Keys);

	PacketProgress = sizeof(GameConfig) - GetStructElementOffset(GameConfig, Buffer);

	RunPacket = (unsigned long long*)LoadedConfig.Buffer;
	LocalEncryption = &Keys[EncStartIndex];
	for (unsigned char i = EncStartIndex; (PacketProgress & ~(sizeof(unsigned long long) - 1)); i++, RunPacket++, LocalEncryption++, PacketProgress -= sizeof(unsigned long long))
	{
		unsigned long long Hash;

		if (i >= GetArraySize(Keys))
		{
			i = 0;
			LocalEncryption = Keys;
		}

		Hash = *LocalEncryption ^ LoadedConfig.Key;

		if (Hash)
			*RunPacket ^= Hash;
		else
			*RunPacket ^= *LocalEncryption;
	}

	if (PacketProgress >= sizeof(unsigned long))
	{
		unsigned long Hash;

		Hash = Keys[5] ^ (LoadedConfig.Key & ((1 << 32) - 1));
		if (Hash)
			(*(unsigned long*)RunPacket) ^= Hash;
		else
			(*(unsigned long*)RunPacket) ^= Keys[0];

		PacketProgress -= sizeof(unsigned long);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned long));
	}

	if (PacketProgress >= sizeof(unsigned short))
	{
		unsigned short Hash;

		Hash = Keys[8] ^ (LoadedConfig.Key & ((1 << 16) - 1));
		if (Hash)
			(*(unsigned short*)RunPacket) ^= Hash;
		else
			(*(unsigned short*)RunPacket) ^= Keys[3];

		PacketProgress -= sizeof(unsigned short);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned short));
	}

	if (PacketProgress >= sizeof(unsigned char))
	{
		unsigned char Hash;

		Hash = Keys[4] ^ (LoadedConfig.Key & ((1 << 8) - 1));
		if (Hash)
			(*(unsigned char*)RunPacket) ^= Hash;
		else
			(*(unsigned char*)RunPacket) ^= Keys[5];

		PacketProgress -= sizeof(unsigned char);
		RunPacket = (unsigned long long*)(((char*)RunPacket) + sizeof(unsigned char));
	}
}

void ConfigManager::UpdateConfig(const char* SteamPath, const char* EpicPath)
{


	OVERLAPPED FileOverlapped;
	HANDLE FileHandle;

	unsigned long Attributes;
	unsigned long Length;
	char Path[MAX_PATH];

	if ((!SteamPath || !strcmp(Instance.LoadedConfig.SteamPath, SteamPath)) && (!EpicPath || !strcmp(Instance.LoadedConfig.EpicPath, EpicPath)))
		return;

	if (SteamPath)
		memcpy(Instance.LoadedConfig.SteamPath, SteamPath, sizeof(Instance.LoadedConfig.SteamPath));

	if (EpicPath)
		memcpy(Instance.LoadedConfig.EpicPath, EpicPath, sizeof(Instance.LoadedConfig.EpicPath));

	SHGetFolderPathA(0, CSIDL_PERSONAL, 0, SHGFP_TYPE_CURRENT, Path);

	Length = strlen(Path);
	memcpy(Path + Length, __CS("\\Archer"), sizeof("\\Archer"));

	Length += sizeof("\\Archer") - 1;

	Attributes = GetFileAttributesA(Path);
	if (Attributes == INVALID_FILE_ATTRIBUTES || !(Attributes & FILE_ATTRIBUTE_DIRECTORY))
		CreateDirectoryA(Path, 0);

	memcpy(Path + Length, __CS("\\Launch.ArchLoad"), sizeof("\\Launch.ArchLoad"));

	FileHandle = CreateFileA(Path, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (FileHandle == INVALID_HANDLE_VALUE)
		return;

	Instance.EncryptConfig();

	memset(&FileOverlapped, 0, sizeof(FileOverlapped));

	FileOverlapped.Offset = sizeof(LoginConfig);
	WriteFile(FileHandle, &Instance.LoadedConfig, sizeof(Instance.LoadedConfig), 0, &FileOverlapped);

	CloseHandle(FileHandle);


}

bool ConfigManager::GetPaths(char* SteamPath, char* EpicPath)
{


	OVERLAPPED FileOverlapped;
	HANDLE FileHandle;

	unsigned long Attributes;
	unsigned long Length;
	char Path[MAX_PATH];

	memset(&Instance.LoadedConfig, 0, sizeof(Instance.LoadedConfig));

	SHGetFolderPathA(0, CSIDL_PERSONAL, 0, SHGFP_TYPE_CURRENT, Path);

	Length = strlen(Path);
	memcpy(Path + Length, __CS("\\Archer"), sizeof("\\Archer"));

	Length += sizeof("\\Archer") - 1;

	Attributes = GetFileAttributesA(Path);
	if (Attributes == INVALID_FILE_ATTRIBUTES || !(Attributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		CreateDirectoryA(Path, 0);
		return false;
	}

	memcpy(Path + Length, __CS("\\Launch.ArchLoad"), sizeof("\\Launch.ArchLoad"));

	FileHandle = CreateFileA(Path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (FileHandle == INVALID_HANDLE_VALUE)
		return false;

	memset(&FileOverlapped, 0, sizeof(FileOverlapped));

	FileOverlapped.Offset = sizeof(LoginConfig);
	if (!ReadFile(FileHandle, &Instance.LoadedConfig, sizeof(Instance.LoadedConfig), 0, &FileOverlapped))
	{
		CloseHandle(FileHandle);
		return false;
	}

	CloseHandle(FileHandle);
	Instance.DecryptConfig();

	if (EpicPath)
		memcpy(EpicPath, Instance.LoadedConfig.EpicPath, sizeof(Instance.LoadedConfig.EpicPath));
	
	if (SteamPath)
		memcpy(SteamPath, Instance.LoadedConfig.SteamPath, sizeof(Instance.LoadedConfig.SteamPath));



	return true;
}