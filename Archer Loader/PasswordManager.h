#pragma once

#define USERNAMESIZE 255
#define PASSWORDSIZE 255

#define SESSIONIDSIZE 32

#define REGISRATIONKEYSIZE 33

class PasswordManager
{
public:
	static bool ProcessLogin();
};