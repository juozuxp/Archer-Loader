#pragma once

template<typename T, bool EraseData = false>
class ArrayBase
{
public:
	class Iterator
	{
	public:
		inline Iterator()
		{
		}

		inline operator T& () const
		{
			return *RunArray;
		}

		inline T* operator->() const
		{
			return RunArray;
		}

		inline T* operator&() const
		{
			return RunArray;
		}

		inline bool operator*() const
		{
			return RunIndex < Base->ArrayNum;
		}

		inline T& operator+(unsigned long Count) const
		{
			return *(RunArray + Count);
		}

		inline T& operator-(unsigned long Count) const
		{
			return *(RunArray - Count);
		}

		inline void Remove()
		{
			Base->Remove(RunIndex);
			RunArray--;
			RunIndex--;
		}

		inline unsigned long GetIndex()
		{
			return RunIndex;
		}

		inline T& operator++()
		{
			RunIndex++;

			if (**this)
				return *++RunArray;

			return *RunArray;
		}

		inline T& operator++(int)
		{
			return operator++();
		}

		inline T& operator--()
		{
			RunIndex--;

			if (**this)
				return *--RunArray;

			return *RunArray;
		}

		inline T& operator--(int)
		{
			return operator--();
		}

		inline T& operator+=(unsigned long Count)
		{
			RunIndex += Count;
			return *(RunArray += Count);
		}

		inline T& operator-=(unsigned long Count)
		{
			RunIndex -= Count;
			return *(RunArray -= Count);
		}

		inline void Reset()
		{
			RunIndex = 0;
			RunArray = Base->TArray;
		}

	private:
		inline Iterator(ArrayBase* Base)
		{
			this->Base = Base;
			this->RunIndex = 0;
			this->RunArray = Base->TArray;
		}

	private:
		friend class ArrayBase;

	private:
		ArrayBase* Base;
		T* RunArray;
		unsigned long RunIndex;
	};

	class ConstIterator
	{
	public:
		inline ConstIterator()
		{
		}

		inline operator T () const
		{
			if (!**this)
				return T();

			return *RunArray;
		}

		inline T* operator->() const
		{
			return RunArray;
		}

		inline T* operator&() const
		{
			return RunArray;
		}

		inline bool operator*() const
		{
			return RunIndex < Base->ArrayNum;
		}

		inline T& operator+(unsigned long Count) const
		{
			return *(RunArray + Count);
		}

		inline T& operator-(unsigned long Count) const
		{
			return *(RunArray - Count);
		}

		inline unsigned long GetIndex()
		{
			return RunIndex;
		}

		inline T& operator++()
		{
			RunIndex++;

			if (**this)
				return *++RunArray;

			return *RunArray;
		}

		inline T& operator++(int)
		{
			return operator++();
		}

		inline T& operator--()
		{
			RunIndex--;

			if (**this)
				return *--RunArray;

			return *RunArray;
		}

		inline T& operator--(int)
		{
			return operator--();
		}

		inline T& operator+=(unsigned long Count)
		{
			RunIndex += Count;
			return *(RunArray += Count);
		}

		inline T& operator-=(unsigned long Count)
		{
			RunIndex -= Count;
			return *(RunArray -= Count);
		}

		inline void Reset()
		{
			RunIndex = 0;
			RunArray = Base->TArray;
		}

	private:
		inline ConstIterator(const ArrayBase* Base)
		{
			this->Base = Base;
			this->RunIndex = 0;
			this->RunArray = Base->TArray;
		}

	private:
		friend class ArrayBase;

	private:
		const ArrayBase* Base;
		T* RunArray;
		unsigned long RunIndex;
	};

public:
	Iterator GetIterator()
	{
		return Iterator(this);
	}

	ConstIterator GetIterator() const
	{
		return ConstIterator(this);
	}

	inline operator T* () const
	{
		return TArray;
	}

	inline T& operator*() const
	{
		return *TArray;
	}

	inline T& operator[](unsigned long i) const
	{
		return TArray[i];
	}

	constexpr unsigned long GetCount() const
	{
		return ArrayNum;
	}

	inline void Remove(const T& Data)
	{
		Iterator Iterator = GetIterator();
		for (T* Current = &Iterator; *Iterator; Current = &Iterator++)
		{
			if (*Current == Data)
			{
				Iterator.Remove();
				break;
			}
		}
	}

	inline void Remove(unsigned long Index)
	{
		Remove(Index, 1);
	}

	inline void Remove(unsigned long Index, unsigned long Count)
	{
		if ((ArrayNum - Index) < Count)
			Count = ArrayNum - Index;

		for (unsigned long i = Index; i < (Index + Count); i++)
			TArray[i].~T();

		ArrayNum -= Count;
		memmove(TArray + Index, TArray + Index + Count, (ArrayNum - Index) * sizeof(T));

		if (EraseData)
			memset(TArray + ArrayNum, 0, Count * sizeof(T));
	}

	inline void Flush()
	{
		for (unsigned long i = 0; i < ArrayNum; i++)
			TArray[i].~T();

		if (EraseData)
			memset(TArray, 0, sizeof(T) * ArrayNum);

		ArrayNum = 0;
	}

protected:
	T* TArray;
	unsigned long ArrayNum;
};