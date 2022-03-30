#pragma once
#include <string.h>
#include <stdlib.h>
#include "ArrayBase.h"

template<typename T, bool EraseData = false>
class DynArray : public ArrayBase<T, EraseData>
{
public:
	inline DynArray()
	{
		this->TArray = 0;
		this->ArrayNum = 0;
		this->ArrayMax = 0;
	}

	inline DynArray(unsigned long BaseAllocation)
	{
		this->ArrayNum = 0;
		this->ArrayMax = BaseAllocation;
		this->TArray = (T*)malloc(BaseAllocation * sizeof(T));

		if (EraseData)
			memset(this->TArray, 0, BaseAllocation * sizeof(T));
	}

	inline DynArray(const T* InitialEntries, unsigned long EntryCount) : DynArray(EntryCount)
	{
		Add(InitialEntries, EntryCount);
	}

	inline DynArray(const DynArray& Second)
	{
		T* RunCopy;
		T* RunAssign;

		this->ArrayNum = Second.ArrayNum;
		this->ArrayMax = Second.ArrayMax;

		this->TArray = (T*)malloc(this->ArrayMax * sizeof(T));

		if (EraseData)
			memset(this->TArray, 0, this->ArrayMax * sizeof(T));

		RunCopy = Second.TArray;
		RunAssign = this->TArray;
		for (unsigned long i = 0; i < this->ArrayNum; i++, RunCopy++, RunAssign++)
			*RunAssign = *RunCopy;
	}

	inline DynArray(DynArray&& Second)
	{
		memcpy(this, &Second, sizeof(Second));
		memset(&Second, 0, sizeof(Second));
	}

	inline ~DynArray()
	{
		if (this->TArray)
		{
			this->Flush();
			free(this->TArray);
		}
	}

	inline DynArray& operator=(const DynArray& Second)
	{
		T* RunCopy;
		T* RunAssign;

		this->ArrayNum = Second.ArrayNum;
		this->ArrayMax = Second.ArrayMax;

		this->TArray = (T*)malloc(this->ArrayMax * sizeof(T));

		if (EraseData)
			memset(this->TArray, 0, this->ArrayMax * sizeof(T));

		RunCopy = Second.TArray;
		RunAssign = this->TArray;
		for (unsigned long i = 0; i < this->ArrayNum; i++, RunCopy++, RunAssign++)
			*RunAssign = *RunCopy;

		return *this;
	}

	inline DynArray& operator=(DynArray&& Second)
	{
		memcpy(this, &Second, sizeof(Second));
		memset(&Second, 0, sizeof(Second));

		return *this;
	}

	inline unsigned long GetMax() const
	{
		return this->ArrayMax;
	}

	inline void SetCount(unsigned long Count)
	{
		if (this->ArrayNum < Count)
			return;

		this->Remove(Count, this->ArrayNum - Count);
	}

	inline void Expand(unsigned long Count)
	{
		if (this->ArrayMax < (this->ArrayNum + Count))
		{
			this->TArray = (T*)realloc(this->TArray, (this->ArrayNum + Count) * sizeof(T));

			if (EraseData)
				memset(this->TArray + this->ArrayMax, 0, ((this->ArrayNum + Count) - this->ArrayMax) * sizeof(T));

			this->ArrayMax = this->ArrayNum + Count;
		}
	}

	inline T* Allocate(unsigned long Count)
	{
		Expand(Count);
		this->ArrayNum += Count;

		return this->TArray + this->ArrayNum - Count;
	}

	inline T& Add(const T& Item)
	{
		Expand(1);

		this->ArrayNum++;

		this->TArray[this->ArrayNum - 1] = Item;
		return this->TArray[this->ArrayNum - 1];
	}

	inline T* Add(const T* Items, unsigned long Count)
	{
		const T* RunOutput;
		T* RunInput;

		Expand(Count);

		this->ArrayNum += Count;

		RunOutput = Items;
		RunInput = &this->TArray[this->ArrayNum - Count];
		for (unsigned long i = 0; i < Count; i++, RunInput++, RunOutput++)
			*RunInput = *RunOutput;

		return &this->TArray[this->ArrayNum - Count];
	}

	inline T* Fill(const T& Item, unsigned long Count)
	{
		T* Return;

		if (!Count)
			return &this->TArray[this->ArrayNum];

		Expand(Count);

		Return = &Add(Item);
		for (unsigned long i = 1; i < Count; i++)
			Add(Item);

		return Return;
	}

	inline T* Fill(const T* Item, unsigned long PerItemCount, unsigned long Count)
	{
		T* Return;

		if (!Count || !PerItemCount)
			return &this->TArray[this->ArrayNum];

		Expand(PerItemCount * Count);

		Return = Add(Item, PerItemCount);
		for (unsigned long i = 1; i < Count; i++)
			Add(Item, PerItemCount);

		return Return;
	}

	inline T& Insert(unsigned long Index, const T& Item)
	{
		Expand(1);

		this->ArrayNum++;

		memcpy(this->TArray + Index + 1, this->TArray + Index, (this->ArrayNum - Index - 1) * sizeof(T));

		if (EraseData)
			memset(&this->TArray[Index], 0, sizeof(T));

		this->TArray[Index] = Item;
		return this->TArray[Index];
	}

	inline T* Insert(unsigned long Index, const T* Items, unsigned long Count)
	{
		Expand(Count);

		this->ArrayNum += Count;

		memcpy(this->TArray + Index + Count, this->TArray + Index, (this->ArrayNum - Index - Count) * sizeof(T));

		if (EraseData)
			memset(&this->TArray[Index], 0, Count * sizeof(T));

		memcpy(&this->TArray[Index], Items, Count * sizeof(T));
		return &this->TArray[Index];
	}

	inline unsigned long long FindIndex(const T& Element)
	{
		T* RunArray;

		RunArray = this->TArray;
		for (unsigned long long i = 0; i < this->ArrayNum; i++, RunArray++)
		{
			if (!memcmp(RunArray, &Element, sizeof(T)))
				return i;
		}

		return ~0;
	}

	inline bool Contains(const T& Element)
	{
		return FindIndex(Element) != ~0ull;
	}

private:
	unsigned long ArrayMax;
};