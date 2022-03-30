#pragma once
#include <Imports.h>

class GrowthBuffer
{
public:
	inline GrowthBuffer();
	inline GrowthBuffer(const GrowthBuffer& Copy);
	inline GrowthBuffer(GrowthBuffer&& Move);

	inline GrowthBuffer(unsigned long InitialLength);

	inline ~GrowthBuffer();

	inline void operator=(const GrowthBuffer& Copy);
	inline void operator=(GrowthBuffer&& Move);

	inline void* Get(unsigned long Length);

private:
	void* Buffer = 0;
	unsigned long Length;
};

GrowthBuffer::GrowthBuffer()
{
}

GrowthBuffer::~GrowthBuffer()
{
	free(Buffer);
}

GrowthBuffer::GrowthBuffer(const GrowthBuffer& Copy)
{
	if (Buffer)
		this->~GrowthBuffer();

	Length = Copy.Length;
	Buffer = malloc(Length);
}

GrowthBuffer::GrowthBuffer(GrowthBuffer&& Move)
{
	if (Buffer)
		this->~GrowthBuffer();

	Length = Move.Length;
	Buffer = malloc(Length);
}

GrowthBuffer::GrowthBuffer(unsigned long InitialLength)
{
	Length = InitialLength;
	Buffer = malloc(InitialLength);
}

void GrowthBuffer::operator=(const GrowthBuffer& Copy)
{
	if (Buffer)
		this->~GrowthBuffer();

	Length = Copy.Length;
	Buffer = malloc(Length);
}

void GrowthBuffer::operator=(GrowthBuffer&& Move)
{
	if (Buffer)
		this->~GrowthBuffer();

	Length = Move.Length;
	Buffer = malloc(Length);
}

void* GrowthBuffer::Get(unsigned long Length)
{
	void* Buffer;

	if (Length <= this->Length)
		return this->Buffer;

	Buffer = malloc(Length);

	free(this->Buffer);

	this->Buffer = Buffer;
	this->Length = Length;

	return Buffer;
}