#pragma once
#include <Imports.h>
#include "PasswordManager.h"

struct LoginConfig
{
	unsigned long long Key;
	union
	{
		char Buffer[1];
		unsigned char SessionID[SESSIONIDSIZE];
	};
};

class ConfigManager
{
public:
	static bool GetSessionID(unsigned char* SessionID);
	static void UpdateConfig(const unsigned char* SessionID);

private:
	static ConfigManager Instance;

private:
	void EncryptConfig();
	void DecryptConfig();

private:
	LoginConfig LoadedConfig;

private:
	static constexpr unsigned long long Keys[] = { 0x7d0a206202e93112, 0x81a6aaeaa5eb8794, 0xbeb9dc171fc1e974, 0x4d353c015efe5c57, 0x11e03c041670b453, 0x03e8d078bfddc175, 0x31f0f2179cd38d44, 0x3078f84e3e32a59d, 0x641cbae9902eb852, 0xfbaa872725ca2860, 0x6713839ca0576b1a, 0x3fd0cd83c049fef8, 0x5edcf268752c3a42, 0x07b9fd079b281b78, 0x0804df5f4896224d };
};