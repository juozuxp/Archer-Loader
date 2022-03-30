#pragma once
#include <dirent.h>
#include <stdio.h>
#include "HashMap.h"
#include "Mutex.h"

class DataBase
{
public:
	template<typename Type>
	class Iterator
	{
	public:
		constexpr Iterator()
		{
		}

		~Iterator()
		{
			closedir(DirectoryHandle);
		}

		Iterator(Iterator&& Move)
		{
			*this = Move;
		}

		Iterator(const Iterator& Copy)
		{
			*this = Copy;
		}

		Iterator& operator=(Iterator&& Move)
		{
			this->FileObject = Move.FileObject;
			this->DirectoryHandle = Move.DirectoryHandle;

			Move.FileObject = 0;
			Move.DirectoryHandle = 0;

			return *this;
		}

		Iterator& operator=(const Iterator& Copy)
		{
			this->FileObject = Copy.FileObject;
			this->DirectoryHandle = Copy.DirectoryHandle;

			return *this;
		}

	public:
		const char* GetEntryName()
		{
			return FileObject->d_name;
		}
	
	public:
		bool operator*()
		{
			return FileObject;
		}

		const Type& operator++()
		{
			RollToNext();
			if (FileObject)
				Parent->ReadEntry<Type>(FileObject->d_name, &ElementBuffer);

			return ElementBuffer;
		}

		const Type& operator++(int)
		{
			return operator++();
		}

		const Type* operator&()
		{
			if (FileObject)
				Parent->ReadEntry<Type>(FileObject->d_name, &ElementBuffer);

			return &ElementBuffer;
		}

		operator const Type& ()
		{
			if (FileObject)
				Parent->ReadEntry<Type>(FileObject->d_name, &ElementBuffer);

			return ElementBuffer;
		}

	private:
		Iterator(DataBase* DataBase)
		{
			char Path[0x1000];

			sprintf(Path, "DataBases/%s", DataBase->Name);

			this->Parent = DataBase;

			this->DirectoryHandle = opendir(Path);
			if (!this->DirectoryHandle)
				return;

			RollToNext();
		}

	private:
		void RollToNext()
		{
			const char* RunName;

			while (true)
			{
				FileObject = readdir(DirectoryHandle);
				if (!FileObject)
					return;
				
				RunName = FileObject->d_name;
				for (; *RunName; RunName++)
				{
					if (*RunName != '.')
						return;
				}
			}
		}

	private:
		DIR* DirectoryHandle = 0;
		struct dirent* FileObject = 0;

	private:
		DataBase* Parent = 0;
		Type ElementBuffer = Type();

	private:
		friend class DataBase;
	};

public:
	DataBase(const char* Name);

	bool DeleteEntry(const char* Name);
	bool ContainEntry(const char* Name);
	bool RenameEntry(const char* Name, const char* NewName);
	bool ReadEntry(const char* Name, void* Buffer, unsigned long Size);
	bool WriteEntry(const char* Name, const void* Buffer, unsigned long Size);

	template<typename T>
	Iterator<T> GetEntryIterator()
	{
		return Iterator<T>(this);
	}

	template<typename T>
	bool ReadEntry(const char* Name, T* Buffer)
	{
		return ReadEntry(Name, Buffer, sizeof(T));
	}

	template<typename T>
	bool WriteEntry(const char* Name, const T& Buffer)
	{
		return WriteEntry(Name, &Buffer, sizeof(T));
	}

private:
	char Name[50];

private:
	HashMap<char, Mutex> Entries = HashMap<char, Mutex>(0);
};

class DataBaseManager
{
public:
	static void Initialize();
	static DataBase* OpenDataBase(const char* Name);

private:
	static DataBaseManager Instance;

private:
	HashMap<char, DataBase*> DataBases = HashMap<char, DataBase*>(0);
};