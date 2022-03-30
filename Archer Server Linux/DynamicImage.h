#pragma once
#include "Pair.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "PseudoRandom.h"

class LongIndex
{
public:
	constexpr LongIndex()
	{
	}

	inline LongIndex(unsigned long long FragmentCount)
	{
		this->FragmentSize = ceil(log(FragmentCount) / log(2));
		this->TableSize = ((this->FragmentSize * FragmentCount) + ((this->FragmentSize * FragmentCount) % 8)) / 8;
		this->IndexBuffer = malloc(this->TableSize);

		memset(this->IndexBuffer, 0, this->TableSize);
	}

	inline LongIndex(unsigned long long FragmentCount, void* Buffer)
	{
		this->FragmentSize = ceil(log(FragmentCount) / log(2));
		this->TableSize = ((this->FragmentSize * FragmentCount) + ((this->FragmentSize * FragmentCount) % 8)) / 8;
		this->IndexBuffer = malloc(this->TableSize);

		memcpy(this->IndexBuffer, Buffer, this->TableSize);
	}

	inline ~LongIndex()
	{
		if (this->IndexBuffer)
			free(this->IndexBuffer);
	}

	inline LongIndex(const LongIndex& Copy)
	{
		this->TableSize = Copy.TableSize;
		this->FragmentSize = Copy.FragmentSize;
		this->IndexBuffer = malloc(this->TableSize);

		memcpy(this->IndexBuffer, 0, this->TableSize);
	}

	inline LongIndex(LongIndex&& Move)
	{
		this->TableSize = Move.TableSize;
		this->FragmentSize = Move.FragmentSize;
		this->IndexBuffer = Move.IndexBuffer;

		Move.IndexBuffer = 0;
	}

	inline LongIndex& operator=(const LongIndex& Copy)
	{
		this->TableSize = Copy.TableSize;
		this->FragmentSize = Copy.FragmentSize;
		this->IndexBuffer = malloc(this->TableSize);

		memcpy(this->IndexBuffer, 0, this->TableSize);

		return *this;
	}

	inline LongIndex& operator=(LongIndex&& Move)
	{
		this->TableSize = Move.TableSize;
		this->FragmentSize = Move.FragmentSize;
		this->IndexBuffer = Move.IndexBuffer;

		Move.IndexBuffer = 0;

		return *this;
	}

	constexpr unsigned long long GetIndexMapSize()
	{
		return TableSize;
	}

	inline void ImportIndexMap(const void* Buffer)
	{
		memcpy(IndexBuffer, Buffer, TableSize);
	}

	inline void ExportIndexMap(void* Buffer)
	{
		memcpy(Buffer, IndexBuffer, TableSize);
	}

	inline void MapIndex(unsigned long long Index, unsigned long long SubIndex)
	{
		unsigned char* LocationInBuffer = ((unsigned char*)IndexBuffer) + ((Index * FragmentSize) / 8ull);

		*LocationInBuffer |= SubIndex << ((Index * FragmentSize) % 8ull);

		LocationInBuffer++;
		for (unsigned char i = 8 - ((Index * FragmentSize) % 8ull); i < FragmentSize; i += 8, LocationInBuffer++)
			*LocationInBuffer |= SubIndex >> i;
	}

	inline unsigned long long operator[](unsigned long long Index)
	{
		unsigned char* LocationInBuffer = ((unsigned char*)IndexBuffer) + ((Index * FragmentSize) / 8ull);
		unsigned long long ExtractedIndex = (((unsigned long long) * LocationInBuffer) >> ((Index * FragmentSize) % 8ull)) & ((1ull << FragmentSize) - 1ull);

		LocationInBuffer++;
		for (unsigned char i = 8 - ((Index * FragmentSize) % 8ull); i < FragmentSize; i += 8, LocationInBuffer++)
			ExtractedIndex |= (((unsigned long long) * LocationInBuffer) & ((1ull << (FragmentSize - i)) - 1ull)) << ((unsigned long long)i);

		return ExtractedIndex;
	}

private:
	void* IndexBuffer = 0;
	unsigned long long TableSize = 0;
	unsigned char FragmentSize = 0;
};

class DynamicImage
{
private:
	struct DynamicDescriptor
	{
		unsigned long long Key;
		unsigned long long SizeKey;
		unsigned char IndexBuffer[0x1000];
	};

public:
	inline DynamicImage()
	{
	}

	DynamicImage(void* ImageBuffer, unsigned long long ImageSize);

public:
	bool StreamImage(class ClientManager& Client);

private:
	void BuildStreamIndex();
	bool DescribeStream(class ClientManager& Client);

	void* GetIndexBuffer(unsigned char Index);
	unsigned char GetIndexFromRandom(bool* UsedFlags);
	unsigned long long GetIndexSize(unsigned char Index);

private:
	void* ImageBuffer = 0;
	unsigned char FragmentCount = 0;
	unsigned long long ImageSize = 0;
	unsigned long long FragmentSize = 0;

	LongIndex IndexMap = LongIndex();
	PseudoRandom Randomizer = PseudoRandom();
};