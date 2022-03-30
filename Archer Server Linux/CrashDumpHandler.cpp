#include "CrashDumpHandler.h"
#include "ClientManager.h"
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <stdlib.h>

#define B_FROM_MB(MBs) ((MBs) * 1024 * 1000)

CrashDumpHandler CrashDumpHandler::Instance;

void CrashDumpHandler::Initialize()
{
	mkdir("Dumps", 0);
}

void CrashDumpHandler::CreateDump(const char* FileNamePrefix, ClientManager& ClientManager, unsigned int Size)
{
	FILE* FileHandle;

	ArcherType Type;
	time_t SnapTime;
	tm* Time;

	char FilePath[PATH_MAX];

	unsigned int Length;
	void* Buffer;

	if (Size > B_FROM_MB(10))
	{
		ClientManager.Send(ArcherType_CrashDump, 0, 0);
		return;
	}

	if (!getcwd(FilePath, sizeof(FilePath)))
	{
		ClientManager.Send(ArcherType_CrashDump, 0, 0);
		return;
	}

	Length = strlen(FilePath);

	SnapTime = time(0);

	Time = localtime(&SnapTime);
	sprintf(FilePath + Length, "/Dumps/%s_%d_%02d_%02d_%02d_%02d_%02d.dmp", FileNamePrefix, Time->tm_year + 1900, Time->tm_mon + 1, Time->tm_mday, Time->tm_hour, Time->tm_min, Time->tm_sec);

	FileHandle = fopen(FilePath, "w");
	if (!FileHandle)
	{
		ClientManager.Send(ArcherType_CrashDump, 0, 0);
		return;
	}

	Buffer = malloc(Size);

	Type = ArcherType_None;
	while (Type != ArcherType_CrashDump)
	{
		Type = ClientManager.Receive(Buffer, Size, 0);
		if (Type == ArcherType_Disconnect)
		{
			free(Buffer);
			fclose(FileHandle);
			remove(FilePath);
			return;
		}
	}

	fwrite(Buffer, sizeof(unsigned char), Size, FileHandle);

	free(Buffer);
	fclose(FileHandle);

	ClientManager.Send(ArcherType_CrashDump, 0, 0);
}