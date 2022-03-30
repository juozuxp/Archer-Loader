#pragma once
#include "DynArray.h"
#include "Array.h"

template<typename Key, typename Type, bool EraseData = false>
class HashMap
{
private:
	class MapLink
	{
	public:
		constexpr MapLink() : FLink(0), BLink(0)
		{
		}

		inline void Link(MapLink* BaseEntry)
		{
			if (FLink && BLink)
				return;

			if (!BaseEntry)
			{
				FLink = this;
				BLink = this;

				return;
			}

			FLink = BaseEntry->FLink;
			BLink = BaseEntry;

			BaseEntry->FLink->BLink = this;
			BaseEntry->FLink = this;
		}

		inline void Unlink()
		{
			if (!FLink || !BLink)
				return;

			BLink->FLink = FLink;
			FLink->BLink = BLink;

			FLink = 0;
			BLink = 0;
		}

		inline MapLink* GetNext() const 
		{
			return FLink;
		}

	private:
		MapLink* FLink;
		MapLink* BLink;
	};

	class MapEntry : public MapLink
	{
	private:
		enum EntryType
		{
			EntryType_None,
			EntryType_Raw,
			EntryType_Batch
		};

	private:
		static Type DefaultType;

	private:
		EntryType EntryType;

	public:
		union EntryData
		{
			class RawEntry
			{
			public:
				inline RawEntry()
				{
				}

				inline RawEntry(unsigned long long Hash)
				{
					this->Hash = Hash;
				}

				inline RawEntry(unsigned long long Hash, const Type& Data)
				{
					this->Hash = Hash;
					this->Data = Data;
				}

				inline operator const Type& () const
				{
					return Data;
				}

				inline operator Type& ()
				{
					return Data;
				}

				inline unsigned long long GetHash() const
				{
					return Hash;
				}

				inline void UpdateData(const Type& Data)
				{
					this->Data = Data;
				}

				inline bool IsEqual(unsigned long long Hash) const
				{
					return this->Hash == Hash;
				}

				inline bool Remove(unsigned long long Hash)
				{
					if (IsEqual(Hash))
					{
						Flush();
						return true;
					}

					return false;
				}

				inline void Flush()
				{
					Data.~Type();
					if (EraseData)
						memset(&Data, 0, sizeof(Data));
				}

			private:
				unsigned long long Hash;
				Type Data = Type();
			} Raw;

			DynArray<RawEntry, EraseData> Batch;
		} Data;

	public:
		inline Type& operator[](unsigned long long Hash)
		{
			return Get(Hash);
		}

		inline const Type& operator[](unsigned long long Hash) const
		{
			return Get(Hash);
		}

		inline operator Array<typename EntryData::RawEntry>()
		{
			switch (this->EntryType)
			{
			case EntryType_Raw:
			{
				return Array<typename EntryData::RawEntry>(&this->Data.Raw, 1);
			} break;
			case EntryType_Batch:
			{
				return Array<typename EntryData::RawEntry>(this->Data.Batch, this->Data.Batch.GetCount());
			} break;
			}

			return Array<typename EntryData::RawEntry>(0, 0);
		}

		inline operator Array<const typename EntryData::RawEntry>() const
		{
			switch (this->EntryType)
			{
			case EntryType_Raw:
			{
				return Array<const typename EntryData::RawEntry>(&this->Data.Raw, 1);
			} break;
			case EntryType_Batch:
			{
				return Array<const typename EntryData::RawEntry>(this->Data.Batch, this->Data.Batch.GetCount());
			} break;
			}

			return Array<const typename EntryData::RawEntry>(0, 0);
		}

		inline Type& Get(unsigned long long Hash)
		{
			switch (this->EntryType)
			{
			case EntryType_Raw:
			{
				if (this->Data.Raw.IsEqual(Hash))
					return this->Data.Raw;

			} break;
			case EntryType_Batch:
			{
				typename DynArray<typename EntryData::RawEntry, EraseData>::Iterator Iterator = this->Data.Batch.GetIterator();

				for (typename EntryData::RawEntry* Current = &Iterator; *Iterator; Current = &Iterator++)
				{
					if (Current->IsEqual(Hash))
						return *Current;
				}
			} break;
			}

			return DefaultType;
		}

		inline const Type& Get(unsigned long long Hash) const
		{
			switch (this->EntryType)
			{
			case EntryType_Raw:
			{
				if (this->Data.Raw.IsEqual(Hash))
					return this->Data.Raw;

			} break;
			case EntryType_Batch:
			{
				typename DynArray<typename EntryData::RawEntry, EraseData>::ConstIterator Iterator = this->Data.Batch.GetIterator();

				for (typename EntryData::RawEntry* Current = &Iterator; *Iterator; Current = &Iterator++)
				{
					if (Current->IsEqual(Hash))
						return *Current;
				}
			} break;
			}

			return DefaultType;
		}

		inline Type& GetByIndex(unsigned int Index)
		{
			switch (this->EntryType)
			{
			case EntryType_Raw:
			{
				return this->Data.Raw;
			} break;
			case EntryType_Batch:
			{
				return this->Data.Batch[Index];
			} break;
			}

			return DefaultType;
		}

		inline const Type& GetByIndex(unsigned int Index) const
		{
			switch (this->EntryType)
			{
			case EntryType_Raw:
			{
				return this->Data.Raw;
			} break;
			case EntryType_Batch:
			{
				return this->Data.Batch[Index];
			} break;
			}

			return DefaultType;
		}

		inline unsigned int GetIndex(unsigned long long Hash) const
		{
			switch (this->EntryType)
			{
			case EntryType_Raw:
			{
				if (this->Data.Raw.IsEqual(Hash))
					return 0;

			} break;
			case EntryType_Batch:
			{
				typename DynArray<typename EntryData::RawEntry, EraseData>::ConstIterator Iterator = this->Data.Batch.GetIterator();

				for (typename EntryData::RawEntry* Current = &Iterator; *Iterator; Current = &Iterator++)
				{
					if (Current->IsEqual(Hash))
						return Iterator.GetIndex();
				}
			} break;
			}

			return ~0;
		}

		inline bool Contains(unsigned long long Hash) const
		{
			return GetIndex(Hash) != ~0;
		}

		inline bool Add(unsigned long long Hash, const Type& Data)
		{
			bool Reserved;

			AddByIndex(ReserveIndex(Hash, &Reserved), Data);

			return Reserved;
		}

		inline bool Remove(unsigned long long Hash)
		{
			switch (this->EntryType)
			{
			case EntryType_Raw:
			{
				if (this->Data.Raw.Remove(Hash))
				{
					this->EntryType = EntryType_None;

					this->Unlink();
					return true;
				}
			} break;
			case EntryType_Batch:
			{
				typename DynArray<typename EntryData::RawEntry, EraseData>::Iterator Iterator = this->Data.Batch.GetIterator();

				for (typename EntryData::RawEntry* Current = &Iterator; *Iterator; Current = &Iterator++)
				{
					if (Current->IsEqual(Hash))
					{
						Iterator.Remove();
						if (!Data.Batch.GetCount())
							this->Unlink();

						return true;
					}
				}
			} break;
			}

			return false;
		}

		inline bool RemoveByIndex(unsigned int Index)
		{
			switch (this->EntryType)
			{
			case EntryType_Raw:
			{
				this->EntryType = EntryType_None;
				this->Data.Raw.Flush();

				this->Unlink();

				return true;
			} break;
			case EntryType_Batch:
			{
				this->Data.Batch.Remove(Index);

				if (!Data.Batch.GetCount())
					this->Unlink();

				return true;
			} break;
			}

			return false;
		}

		inline void Flush()
		{
			switch (this->EntryType)
			{
			case EntryType_Raw:
			{
				this->EntryType = EntryType_None;
				this->Data.Raw.Flush();

				this->Unlink();
			} break;
			case EntryType_Batch:
			{
				this->Data.Batch.Flush();
				this->Unlink();
			} break;
			}
		}

		inline void Destroy()
		{
			Flush();
			if (this->EntryType == EntryType_Batch)
				this->Data.Batch.~DynArray();
		}

		inline void AddByIndex(unsigned int Index, const Type& Data)
		{
			switch (this->EntryType)
			{
			case EntryType_Raw:
			{
				this->Data.Raw.UpdateData(Data);
			} break;
			case EntryType_Batch:
			{
				this->Data.Batch[Index].UpdateData(Data);
			} break;
			}
		}

		inline unsigned int ReserveIndex(unsigned long long Hash, bool* Reserved = 0)
		{
			switch (this->EntryType)
			{
			case EntryType_None:
			{
				if (Reserved)
					*Reserved = true;

				this->EntryType = EntryType_Raw;
				this->Data.Raw = typename EntryData::RawEntry(Hash);

				return 0;
			} break;
			case EntryType_Raw:
			{
				if (this->Data.Raw.IsEqual(Hash))
				{
					if (Reserved)
						*Reserved = false;

					return 0;
				}

				if (Reserved)
					*Reserved = true;

				typename EntryData::RawEntry* LinkBase;
				typename EntryData::RawEntry CopyData;

				CopyData = this->Data.Raw;

				this->EntryType = EntryType_Batch;

				this->Data.Batch = DynArray<typename EntryData::RawEntry, EraseData>(&CopyData, 1);
				this->Data.Batch.Add(typename EntryData::RawEntry(Hash));

				return 1;
			} break;
			case EntryType_Batch:
			{
				typename DynArray<typename EntryData::RawEntry, EraseData>::Iterator Iterator = this->Data.Batch.GetIterator();

				for (typename EntryData::RawEntry* Current = &Iterator; *Iterator; Current = &Iterator++)
				{
					if (Current->IsEqual(Hash))
					{
						if (Reserved)
							*Reserved = false;

						return Iterator.GetIndex();
					}
				}

				if (Reserved)
					*Reserved = true;

				this->Data.Batch.Add(typename EntryData::RawEntry(Hash));
				return this->Data.Batch.GetCount() - 1;
			} break;
			}

			if (Reserved)
				*Reserved = false;
		}
	};

public:
	class Iterator
	{
	public:
		inline Iterator()
		{
		}

		inline operator Type& () const
		{
			if (!**this)
				return Default;

			return CurIterator->operator Type & ();
		}

		inline Type* operator->() const
		{
			if (!**this)
				return &Default;

			return &CurIterator->operator Type & ();
		}

		inline Type* operator&() const
		{
			if (!**this)
				return &Default;

			return &CurIterator->operator Type & ();
		}

		inline bool operator*() const
		{
			return RunMap != &Base->BaseEntry;
		}

		/*inline T& operator+(unsigned int Count) const
		{
			return *(RunArray + Count);
		}

		inline T& operator-(unsigned int Count) const
		{
			return *(RunArray - Count);
		}*/

		/*inline void Remove()
		{
			if (!**this)
				return;

			if (BaseArray.GetCount() == 1)

			Base->RemoveByHash(CurIterator.);
			RunArray--;
			RunIndex--;
		}*/

		/*inline unsigned int GetIndex()
		{
			return RunIndex;
		}*/

		inline Type& operator++()
		{
			if (**this)
			{
				Type* Return;

				Return = &(++CurIterator).operator Type & ();
				if (!*CurIterator)
				{
					RunMap = (MapEntry*)RunMap->GetNext();

					if (**this)
					{
						this->BaseArray = this->RunMap->operator Array<typename MapEntry::EntryData::RawEntry>();
						this->CurIterator = BaseArray.GetIterator();
					}

					return CurIterator->operator Type & ();
				}

				return *Return;
			}

			return Default;
		}

		inline Type& operator++(int)
		{
			return operator++();
		}

		/*inline T& operator+=(unsigned int Count)
		{
			RunIndex += Count;
			return *(RunArray += Count);
		}

		inline T& operator-=(unsigned int Count)
		{
			RunIndex -= Count;
			return *(RunArray -= Count);
		}*/

		/*inline void Reset()
		{
			RunIndex = 0;
			RunArray = Base->TArray;
		}*/

	private:
		inline Iterator(HashMap* Base)
		{
			this->Base = Base;
			this->RunMap = (MapEntry*)Base->BaseEntry.GetNext();

			if (this->RunMap != &Base->BaseEntry)
			{
				this->BaseArray = this->RunMap->operator Array<typename MapEntry::EntryData::RawEntry>();
				this->CurIterator = BaseArray.GetIterator();
			}
		}

	private:
		friend class HashMap;

	private:
		static Type Default;

		HashMap* Base;
		MapEntry* RunMap;
		Array<typename MapEntry::EntryData::RawEntry> BaseArray;
		typename Array<typename MapEntry::EntryData::RawEntry>::Iterator CurIterator;
	};

public:
	inline HashMap()
	{
	}

	inline ~HashMap()
	{
		Flush();
		if (MapEntries)
			free(MapEntries);

		MapEntries = 0;
	}

	inline HashMap(unsigned char IndexBits)
	{
		this->EntryCount = 0;
		this->BaseEntry.Link(0);
		this->IndexBits = IndexBits;
		this->MapEntries = (MapEntry*)malloc((1 << IndexBits) * sizeof(MapEntry));

		memset(this->MapEntries, 0, (1 << IndexBits) * sizeof(MapEntry));
	}

	inline HashMap(const HashMap& Second) : HashMap(Second.IndexBits)
	{
		for (MapEntry* Current = (MapEntry*)Second.BaseEntry.GetNext(); Current != &Second.BaseEntry; Current = (MapEntry*)Current->GetNext())
		{
			Array<typename MapEntry::EntryData::RawEntry> Array = Current->operator ::Array<typename MapEntry::EntryData::RawEntry>();
			typename ::Array<typename MapEntry::EntryData::RawEntry>::Iterator Iterator = Array.GetIterator();

			for (typename MapEntry::EntryData::RawEntry* RawEntry = &Iterator; *Iterator; RawEntry = &Iterator++)
				AddByHash(RawEntry->GetHash(), *RawEntry);
		}
	}

	inline HashMap(HashMap&& Second) : HashMap(Second.IndexBits)
	{
		for (MapEntry* Current = (MapEntry*)Second.BaseEntry.GetNext(); Current != &Second.BaseEntry; Current = (MapEntry*)Current->GetNext())
		{
			Array<typename MapEntry::EntryData::RawEntry> Array = Current->operator ::Array<typename MapEntry::EntryData::RawEntry>();
			typename ::Array<typename MapEntry::EntryData::RawEntry>::Iterator Iterator = Array.GetIterator();

			for (typename MapEntry::EntryData::RawEntry* RawEntry = &Iterator; *Iterator; RawEntry = &Iterator++)
				AddByHash(RawEntry->GetHash(), *RawEntry);
		}

		Second.Flush();
		free(Second.MapEntries);
		Second.MapEntries = 0;
	}

	inline HashMap& operator=(const HashMap& Second)
	{
		EntryCount = 0;
		BaseEntry.Link(0);
		IndexBits = IndexBits;
		MapEntries = (MapEntry*)malloc((1 << IndexBits) * sizeof(MapEntry));

		memset(this->MapEntries, 0, (1 << IndexBits) * sizeof(MapEntry));

		for (MapEntry* Current = (MapEntry*)Second.BaseEntry.GetNext(); Current != &Second.BaseEntry; Current = (MapEntry*)Current->GetNext())
		{
			Array<typename MapEntry::EntryData::RawEntry> Array = Current->operator ::Array<typename MapEntry::EntryData::RawEntry>();
			typename ::Array<typename MapEntry::EntryData::RawEntry>::Iterator Iterator = Array.GetIterator();

			for (typename MapEntry::EntryData::RawEntry* RawEntry = &Iterator; *Iterator; RawEntry = &Iterator++)
				AddByHash(RawEntry->GetHash(), *RawEntry);
		}

		return *this;
	}

	inline HashMap& operator=(HashMap&& Second)
	{
		EntryCount = 0;
		BaseEntry.Link(0);
		IndexBits = IndexBits;
		MapEntries = (MapEntry*)malloc((1 << IndexBits) * sizeof(MapEntry));

		memset(this->MapEntries, 0, (1 << IndexBits) * sizeof(MapEntry));

		for (MapEntry* Current = (MapEntry*)Second.BaseEntry.GetNext(); Current != &Second.BaseEntry; Current = (MapEntry*)Current->GetNext())
		{
			Array<typename MapEntry::EntryData::RawEntry> Array = Current->operator ::Array<typename MapEntry::EntryData::RawEntry>();
			typename ::Array<typename MapEntry::EntryData::RawEntry>::Iterator Iterator = Array.GetIterator();

			for (typename MapEntry::EntryData::RawEntry* RawEntry = &Iterator; *Iterator; RawEntry = &Iterator++)
				AddByHash(RawEntry->GetHash(), *RawEntry);
		}

		Second.Flush();
		free(Second.MapEntries);
		Second.MapEntries = 0;

		return *this;
	}

	inline Iterator GetIterator()
	{
		return Iterator(this);
	}

	inline Type& operator[](const Key& Index)
	{
		return Get(Index);
	}

	inline const Type& operator[](const Key& Index) const
	{
		return Get(Index);
	}

	inline Type& Get(const Key& Index)
	{
		unsigned long long Hash;

		Hash = Create64Hash(&Index, sizeof(Key));
		return MapEntries[GetHashIndex(Hash)][Hash];
	}

	constexpr unsigned long long GetCount()
	{
		return EntryCount;
	}

	inline Type& Get(const Key* Index, unsigned int IndexSize)
	{
		unsigned long long Hash;

		Hash = Create64Hash(Index, IndexSize * sizeof(Key));
		return MapEntries[GetHashIndex(Hash)][Hash];
	}

	inline const Type& Get(const Key& Index) const
	{
		unsigned long long Hash;

		Hash = Create64Hash(&Index, sizeof(Key));
		return MapEntries[GetHashIndex(Hash)][Hash];
	}

	inline const Type& Get(const Key* Index, unsigned int IndexSize) const
	{
		unsigned long long Hash;

		Hash = Create64Hash(Index, IndexSize * sizeof(Key));
		return MapEntries[GetHashIndex(Hash)][Hash];
	}

	inline bool Contains(const Key& Index) const
	{
		unsigned long long Hash;

		Hash = Create64Hash(&Index, sizeof(Key));
		return MapEntries[GetHashIndex(Hash)].Contains(Hash);
	}

	inline bool Contains(const Key* Index, unsigned int IndexSize) const
	{
		unsigned long long Hash;

		Hash = Create64Hash(Index, IndexSize * sizeof(Key));
		return MapEntries[GetHashIndex(Hash)].Contains(Hash);
	}

	inline Type& GetByIndex(unsigned long long Index)
	{
		return MapEntries[Index & ((1ull << 32) - 1)].GetByIndex(Index >> 32);
	}

	inline const Type& GetByIndex(unsigned long long Index) const
	{
		return MapEntries[Index & ((1ull << 32) - 1)].GetByIndex(Index >> 32);
	}

	inline unsigned long long GetIndex(const Key& Index) const
	{
		unsigned long long EntryIndex;
		unsigned long long HashIndex;
		unsigned long long Hash;

		Hash = Create64Hash(&Index, sizeof(Key));
		HashIndex = GetHashIndex(Hash);

		EntryIndex = MapEntries[HashIndex].GetIndex(Hash);
		if (((unsigned int)EntryIndex) == ~0)
			return ~0;

		return HashIndex | (EntryIndex << 32);
	}

	inline unsigned long long GetIndex(const Key* Index, unsigned int IndexSize) const
	{
		unsigned long long EntryIndex;
		unsigned long long HashIndex;
		unsigned long long Hash;

		Hash = Create64Hash(Index, IndexSize * sizeof(Key));
		HashIndex = GetHashIndex(Hash);

		EntryIndex = MapEntries[HashIndex].GetIndex(Hash);
		if (((unsigned int)EntryIndex) == ~0)
			return ~0;

		return HashIndex | (EntryIndex << 32);
	}

	inline unsigned long long ReserveIndex(const Key& Index, bool* Reserved = 0)
	{
		unsigned long long EntryIndex;
		unsigned long long HashIndex;
		unsigned long long Hash;

		Hash = Create64Hash(&Index, sizeof(Key));
		HashIndex = GetHashIndex(Hash);

		EntryIndex = MapEntries[HashIndex].GetIndex(Hash);
		if (((unsigned int)EntryIndex) == ~0)
		{
			EntryCount++;
			if (EntryCount > (1 << IndexBits))
			{
				Resize(IndexBits + 1);
				HashIndex = GetHashIndex(Hash);
			}

			EntryIndex = MapEntries[HashIndex].ReserveIndex(Hash);
			MapEntries[HashIndex].Link(&BaseEntry);

			if (Reserved)
				*Reserved = true;
		}
		else
		{
			if (Reserved)
				*Reserved = false;
		}

		return HashIndex | (EntryIndex << 32);
	}

	inline unsigned long long ReserveIndex(const Key* Index, unsigned int IndexSize, bool* Reserved = 0)
	{
		unsigned long long EntryIndex;
		unsigned long long HashIndex;
		unsigned long long Hash;

		Hash = Create64Hash(Index, IndexSize * sizeof(Key));
		HashIndex = GetHashIndex(Hash);

		EntryIndex = MapEntries[HashIndex].GetIndex(Hash);
		if (((unsigned int)EntryIndex) == ~0)
		{
			EntryCount++;
			if (EntryCount > (1 << IndexBits))
			{
				Resize(IndexBits + 1);
				HashIndex = GetHashIndex(Hash);
			}

			EntryIndex = MapEntries[HashIndex].ReserveIndex(Hash);
			MapEntries[HashIndex].Link(&BaseEntry);

			if (Reserved)
				*Reserved = true;
		}
		else
		{
			if (Reserved)
				*Reserved = false;
		}

		return HashIndex | (EntryIndex << 32);
	}

	inline void Add(const Key& Index, const Type& Data)
	{
		AddByHash(Create64Hash(&Index, sizeof(Key)), Data);
	}

	inline void Add(const Key* Index, unsigned int IndexSize, const Type& Data)
	{
		AddByHash(Create64Hash(Index, IndexSize * sizeof(Key)), Data);
	}

	inline void Remove(const Key& Index)
	{
		RemoveByHash(Create64Hash(&Index, sizeof(Key)));
	}

	inline void Remove(const Key* Index, unsigned int IndexSize)
	{
		RemoveByHash(Create64Hash(Index, IndexSize * sizeof(Key)));
	}

	inline void RemoveByIndex(unsigned long long Index)
	{
		if (MapEntries[Index & ((1ull << 32) - 1)].RemoveByIndex(Index >> 32))
			EntryCount--;
	}

	inline void Flush()
	{
		EntryCount = 0;
		for (MapEntry* Current = (MapEntry*)BaseEntry.GetNext(),* NextElement = (MapEntry*)Current->GetNext(); Current != &BaseEntry; Current = NextElement, NextElement = (MapEntry*)Current->GetNext())
			Current->Flush();
	}

private:
	inline void RemoveByHash(unsigned long long Hash)
	{
		if (MapEntries[GetHashIndex(Hash)].Remove(Hash))
			EntryCount--;
	}

	inline void Resize(unsigned int NewIndexBits)
	{
		MapEntry* OldMap;

		OldMap = MapEntries;
		IndexBits = NewIndexBits;
		MapEntries = (MapEntry*)malloc((1ul << NewIndexBits) * sizeof(MapEntry));

		memset(MapEntries, 0, (1ul << NewIndexBits) * sizeof(MapEntry));

		for (MapEntry* Current = (MapEntry*)BaseEntry.GetNext(); Current != &BaseEntry; )
		{
			MapEntry* NextEntry;

			NextEntry = (MapEntry*)Current->GetNext();
			if (Current >= MapEntries && Current <= MapEntries + (1ul << IndexBits))
			{
				Current = NextEntry;
				continue;
			}

			Array<typename MapEntry::EntryData::RawEntry> Array = Current->operator ::Array<typename MapEntry::EntryData::RawEntry>();
			typename ::Array<typename MapEntry::EntryData::RawEntry>::Iterator Iterator = Array.GetIterator();

			for (typename MapEntry::EntryData::RawEntry* RawEntry = &Iterator; *Iterator; RawEntry = &Iterator++)
			{
				EntryCount--;
				AddByHash(RawEntry->GetHash(), *RawEntry);
			}

			Current->Destroy();
			Current = NextEntry;
		}

		free(OldMap);
	}

	inline void AddByHash(unsigned long long Hash, const Type& Data)
	{
		unsigned int HashIndex;

		HashIndex = GetHashIndex(Hash);
		if (MapEntries[HashIndex].Add(Hash, Data))
		{
			MapEntries[HashIndex].Link(&BaseEntry);

			EntryCount++;
			if (EntryCount > (1ul << IndexBits))
				Resize(IndexBits + 1);
		}
	}

	inline unsigned int GetHashIndex(unsigned long long Hash) const
	{
		return Hash & ((1ul << IndexBits) - 1);
	}

	inline unsigned long long Create64Hash(const void* Buffer, unsigned int BufferSize) const
	{
		unsigned long long Hash64 = 14695981039346656037;
		const unsigned int Prime = 591798841;

		unsigned long long Cycles;
		unsigned long long NDhead;

		const char* RunKey = (const char*)Buffer;
		if (BufferSize > 8)
		{
			Cycles = ((BufferSize - 1) >> 4) + 1;
			NDhead = BufferSize - (Cycles << 3);

			for (; Cycles--; RunKey += 8)
			{
				Hash64 = (Hash64 ^ (*(unsigned long long*)(RunKey))) * Prime;
				Hash64 = (Hash64 ^ (*(unsigned long long*)(RunKey + NDhead))) * Prime;
			}
		}
		else
			Hash64 = (Hash64 ^ ((*(unsigned long long*)(RunKey) << ((8 - BufferSize) << 3)) >> ((8 - BufferSize) << 3))) * Prime;

		return Hash64;
	}

private:
	unsigned char IndexBits;
	unsigned long long EntryCount;
	MapEntry* MapEntries;
	MapLink BaseEntry;
};

template<typename Key, typename Type, bool EraseData>
Type HashMap<Key, Type, EraseData>::MapEntry::DefaultType = Type();

template<typename Key, typename Type, bool EraseData>
Type HashMap<Key, Type, EraseData>::Iterator::Default = Type();