#pragma once
#include "Pair.h"
#include <BasicUtilities.h>
#include <string.h>
#include <intrin.h>
#include <math.h>

#ifndef _DEBUG
#include <ThemidaSDK.h>
#endif

class RSA
{
public:
	constexpr RSA()
	{
	}

	constexpr RSA(Pair<unsigned long, unsigned long> KeyPair) : EncryptKey(KeyPair.First), EncryptLength(KeyPair.Second)
	{
	}

	void Encrypt(const void* Buffer, unsigned long long Size, void** EncryptedBuffer, unsigned long long* EncryptedSize)
	{
		unsigned long long ResultIndex;
		unsigned long long InfoIndex;
		unsigned long long Value;

		unsigned char LengthBits;
		unsigned char ResultBits;

#ifndef _DEBUG
		VM_START
#endif

		ResultBits = ceil(log(EncryptLength) / log(2));
		LengthBits = floor(log(EncryptLength) / log(2));

		Value = ((Size * 8) / ((unsigned long long)LengthBits)) + !!((Size * 8) % ((unsigned long long)LengthBits));
		*EncryptedSize = ((Value * ((unsigned long long)ResultBits)) / 8) + !!((Value * ((unsigned long long)ResultBits)) % 8);

		*EncryptedBuffer = malloc(*EncryptedSize);
		memset(*EncryptedBuffer, 0, *EncryptedSize);
		for (InfoIndex = 0, ResultIndex = 0; InfoIndex < (Size * 8) - ((Size * 8) % ((unsigned long long)LengthBits)); InfoIndex += LengthBits, ResultIndex += ResultBits)
		{
			Value = GetBitOffsetValue(Buffer, InfoIndex, LengthBits);
			SetBitOffsetValue(*EncryptedBuffer, ResultIndex, ResultBits, ExpMod(Value, EncryptKey, EncryptLength));
		}

		if ((Size * 8) % ((unsigned long long)LengthBits))
		{
			Value = GetBitOffsetValue(Buffer, InfoIndex, (Size * 8) % ((unsigned long long)LengthBits));
			SetBitOffsetValue(*EncryptedBuffer, ResultIndex, ResultBits, ExpMod(Value, EncryptKey, EncryptLength));

#ifndef _DEBUG
			VM_END
#endif
		}
	}

private:
	static constexpr unsigned long long GetBitOffsetValue(const void* Buffer, unsigned long long BitOffset, unsigned long long BitSize)
	{
		unsigned char* LocationInBuffer = ((unsigned char*)Buffer) + (BitOffset / 8ull);
		unsigned long long ExtractedValue = (((unsigned long long) * LocationInBuffer) >> (BitOffset % 8ull)) & ((1ull << BitSize) - 1ull);

		LocationInBuffer++;
		for (unsigned char i = 8 - (BitOffset % 8ull); i < BitSize; i += 8, LocationInBuffer++)
			ExtractedValue |= (((unsigned long long) * LocationInBuffer) & ((1ull << (BitSize - i)) - 1ull)) << ((unsigned long long)i);

		return ExtractedValue;
	}

	static constexpr void SetBitOffsetValue(void* Buffer, unsigned long long BitOffset, unsigned long long BitSize, unsigned long long Value)
	{
		unsigned char* LocationInBuffer = ((unsigned char*)Buffer) + (BitOffset / 8ull);

		*LocationInBuffer |= Value << (BitOffset % 8ull);

		LocationInBuffer++;
		for (unsigned char i = 8 - (BitOffset % 8ull); i < BitSize; i += 8, LocationInBuffer++)
			*LocationInBuffer |= Value >> i;
	}

	unsigned long long ExpMod(unsigned long long Num, unsigned long long Exp, unsigned long long Mod)
	{
		if (Exp == 1)
			return Num % Mod;

		unsigned long long Variable;
		if (Exp % 2)
		{
			Variable = ExpMod(Num, (Exp - 1) / 2, Mod);
			return (Num * Variable * Variable) % Mod;
		}

		Variable = ExpMod(Num, Exp / 2, Mod);

		return (Variable * Variable) % Mod;
	}

private:
	unsigned long EncryptKey = 0;
	unsigned long EncryptLength = 0;
};