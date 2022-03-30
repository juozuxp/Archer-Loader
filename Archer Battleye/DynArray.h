#pragma once
#include <Imports.h>
#include "ArrayBase.h"

template<typename T, bool EraseData = false>
class DynArray : public ArrayBase<T, EraseData>
{
public:
	inline DynArray()
	{
		this->TArray = 0;
	}

	inline DynArray(unsigned long BaseAllocation)
	{
		this->ArrayNum = 0;
		this->ArrayMax = BaseAllocation;
		this->TArray = (T*)malloc(BaseAllocation * sizeof(T));

		if (EraseData)
			memset(this->TArray, 0, BaseAllocation * sizeof(T));
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

	inline void Expand(unsigned long Allocation)
	{
		if (ArrayMax < (this->ArrayNum + Allocation))
		{
			this->TArray = (T*)realloc(this->TArray, (this->ArrayNum + Allocation) * sizeof(T));

			if (EraseData)
				memset(this->TArray + this->ArrayMax, 0, ((this->ArrayNum + Allocation) - this->ArrayMax) * sizeof(T));

			this->ArrayMax = this->ArrayNum + Allocation;
		}
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
		Expand(Count);

		this->ArrayNum += Count;

		memcpy(&this->TArray[this->ArrayNum - Count], Items, Count * sizeof(T));
		return &this->TArray[this->ArrayNum - Count];
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

private:
	unsigned long ArrayMax;
};