#pragma once

class CrashDumpHandler
{
public:
	void Initialize();
	void CreateDump(const char* FileNamePrefix, class ClientManager& ClientManager, unsigned int Size);

public:
	static CrashDumpHandler Instance;
};

