#include "DataBaseManager.h"
#include "Utilities.h"
#include <sys/stat.h>
#include <stdio.h>

DataBaseManager DataBaseManager::Instance;

DataBase::DataBase(const char* Name)
{
	sprintf(this->Name, "%s", Name);
}

bool DataBase::ContainEntry(const char* Name)
{
	unsigned long long DataBaseIndex;
	bool IndexReserved;

	char Path[0x1000];

	FILE* FileHandle;

	DataBaseIndex = Entries.ReserveIndex(Name, strlen(Name), &IndexReserved);
	if (IndexReserved)
		Entries.GetByIndex(DataBaseIndex) = Mutex();

	Entries.GetByIndex(DataBaseIndex).WaitForLock();

	sprintf(Path, "DataBases/%s/%s", this->Name, Name);

	FileHandle = fopen(Path, "r");
	if (!FileHandle)
	{
		Entries.GetByIndex(DataBaseIndex).Free();
		return false;
	}

	fclose(FileHandle);

	Entries.GetByIndex(DataBaseIndex).Free();
	return true;
}

bool DataBase::RenameEntry(const char* Name, const char* NewName)
{
	unsigned long long DataBaseIndex;

	bool Renamed;

	char NewPath[0x1000];
	char Path[0x1000];

	DataBaseIndex = Entries.GetIndex(Name, strlen(Name));
	if (DataBaseIndex == ~0ull)
		return false;

	if (ContainEntry(NewName))
		return false;

	Entries.GetByIndex(DataBaseIndex).WaitForLock();
	
	sprintf(Path, "DataBases/%s/%s", this->Name, Name);
	sprintf(NewPath, "DataBases/%s/%s", this->Name, NewName);

	Renamed = !rename(Path, NewPath);

	Entries.GetByIndex(DataBaseIndex).Free();

	return Renamed;
}

bool DataBase::DeleteEntry(const char* Name)
{
	unsigned long long DataBaseIndex;
	bool IndexReserved;

	char Path[0x1000];

	DataBaseIndex = Entries.ReserveIndex(Name, strlen(Name), &IndexReserved);
	if (IndexReserved)
		Entries.GetByIndex(DataBaseIndex) = Mutex();

	Entries.GetByIndex(DataBaseIndex).WaitForLock();

	sprintf(Path, "DataBases/%s/%s", this->Name, Name);

	IndexReserved = !remove(Path);

	Entries.GetByIndex(DataBaseIndex).Free();
	return IndexReserved;
}

bool DataBase::ReadEntry(const char* Name, void* Buffer, unsigned long Size)
{
	unsigned long long DataBaseIndex;
	bool IndexReserved;

	char Path[0x1000];

	FILE* FileHandle;

	DataBaseIndex = Entries.ReserveIndex(Name, strlen(Name), &IndexReserved);
	if (IndexReserved)
		Entries.GetByIndex(DataBaseIndex) = Mutex();

	Entries.GetByIndex(DataBaseIndex).WaitForLock();

	sprintf(Path, "DataBases/%s/%s", this->Name, Name);
	
	FileHandle = fopen(Path, "r");
	if (!FileHandle)
	{
		Entries.GetByIndex(DataBaseIndex).Free();
		return false;
	}

	memset(Buffer, 0, Size);
	if (Size && !fread(Buffer, Size, 1, FileHandle))
	{
		fclose(FileHandle);
		Entries.GetByIndex(DataBaseIndex).Free();
		return false;
	}

	fclose(FileHandle);

	Entries.GetByIndex(DataBaseIndex).Free();
	return true;
}

bool DataBase::WriteEntry(const char* Name, const void* Buffer, unsigned long Size)
{
	unsigned long long DataBaseIndex;
	bool IndexReserved;

	char Path[0x1000];

	FILE* FileHandle;

	DataBaseIndex = Entries.ReserveIndex(Name, strlen(Name), &IndexReserved);
	if (IndexReserved)
		Entries.GetByIndex(DataBaseIndex) = Mutex();

	Entries.GetByIndex(DataBaseIndex).WaitForLock();

	sprintf(Path, "DataBases/%s/%s", this->Name, Name);

	FileHandle = fopen(Path, "w");
	if (!FileHandle)
	{
		Entries.GetByIndex(DataBaseIndex).Free();
		return false;
	}

	if (Size && !fwrite(Buffer, Size, 1, FileHandle))
	{
		fclose(FileHandle);
		Entries.GetByIndex(DataBaseIndex).Free();
		return false;
	}

	fclose(FileHandle);

	Entries.GetByIndex(DataBaseIndex).Free();
	return true;
}

void DataBaseManager::Initialize()
{
	mkdir("DataBases", S_IRWXU);
}

DataBase* DataBaseManager::OpenDataBase(const char* Name)
{
	unsigned long long DataBaseIndex;
	bool IndexReserved;

	char Path[0x1000];

	DataBase* OpenedDataBase;
	DIR* DirectoryHandle;

	DataBaseIndex = Instance.DataBases.ReserveIndex(Name, strlen(Name), &IndexReserved);
	if (!IndexReserved)
		return Instance.DataBases.GetByIndex(DataBaseIndex);

	sprintf(Path, "DataBases/%s", Name);
	mkdir(Path, S_IRWXU);

	OpenedDataBase = (DataBase*)malloc(sizeof(DataBase));
	if (!OpenedDataBase)
		return 0;

	*OpenedDataBase = DataBase(Name);
	Instance.DataBases.GetByIndex(DataBaseIndex) = OpenedDataBase;

	return OpenedDataBase;
}