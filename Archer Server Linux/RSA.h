#pragma once
#include "Pair.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "time.h"
#include "Randomizer.h"

#define GetArraySize(Array) ((sizeof(Array)) / (sizeof(Array[0])))

class RSA
{
public:
	constexpr RSA()
	{
	}

	unsigned long long CryptSize(unsigned long long Size)
	{
		unsigned long long ByteCount;

		unsigned char LengthBits;
		unsigned char ResultBits;

		ResultBits = ceil(log(DecryptLength) / log(2));
		LengthBits = floor(log(DecryptLength) / log(2));

		ByteCount = ((Size * 8) / ((unsigned long long)LengthBits)) + !!((Size * 8) % ((unsigned long long)LengthBits));
		return ((ByteCount * ((unsigned long long)ResultBits)) / 8) + !!((ByteCount * ((unsigned long long)ResultBits)) % 8);
	}

	Pair<unsigned int, unsigned int> GenerateEncryptionKeys()
	{
		Pair<unsigned int, unsigned int> KeyPair;

		unsigned int LastCoPrime;
		unsigned int KIncrament;
		unsigned short Random;
		unsigned int Second;
		unsigned int First;
		unsigned int Phi;

		First = Randomizer::RandomNumber() % GetArraySize(Primes);
		while (true)
		{
			Second = Randomizer::RandomNumber() % GetArraySize(Primes);
			if (Second != First)
				break;
		}

		First = Primes[First];
		Second = Primes[Second];

		DecryptLength = First * Second;

		KeyPair.Element2 = DecryptLength;
		Phi = (First - 1) * (Second - 1);

		Random = Randomizer::RandomNumber();

		DecryptKey = 0;
		LastCoPrime = 0;
		KeyPair.Element1 = 2;
		while (!DecryptKey)
		{
			if (LastCoPrime == KeyPair.Element1)
				KeyPair.Element1 = 2;

			for (KeyPair.Element1++;; KeyPair.Element1++)
			{
				if (KeyPair.Element1 >= Phi)
					KeyPair.Element1 = LastCoPrime;

				if (IsCoprime(KeyPair.Element1, Phi))
				{
					if (!Random)
						break;

					LastCoPrime = KeyPair.Element1;
					Random--;
				}
			}

			DecryptKey = GenerateDecrypt(Phi, KeyPair.Element1);
		}

		return KeyPair;
	}

	void Decrypt(const void* Buffer, unsigned long long Size, void** DecryptBuffer, unsigned long long* DecryptSize)
	{
		unsigned long long ResultBytes;
		unsigned long long ResultIndex;
		unsigned long long InfoIndex;
		unsigned long long Value;

		unsigned char LengthBits;
		unsigned char ResultBits;

		ResultBits = ceil(log(DecryptLength) / log(2));
		LengthBits = floor(log(DecryptLength) / log(2));

		ResultBytes = (Size * 8) / ((unsigned long long)ResultBits);
		Value = ((ResultBytes * ((unsigned long long)LengthBits)) / 8) + !!((ResultBytes * ((unsigned long long)LengthBits)) % 8);
		
		if (DecryptSize)
			*DecryptSize = Value;

		*DecryptBuffer = malloc(Value);
		memset(*DecryptBuffer, 0, Value);
		for (InfoIndex = 0, ResultIndex = 0; ResultIndex < (ResultBytes * ResultBits); InfoIndex += LengthBits, ResultIndex += ResultBits)
		{
			Value = GetBitOffsetValue(Buffer, ResultIndex, ResultBits);
			SetBitOffsetValue(*DecryptBuffer, InfoIndex, LengthBits, ExpMod(Value, DecryptKey, DecryptLength));
		}
	}

private:
	static unsigned long long GetBitOffsetValue(const void* Buffer, unsigned long long BitOffset, unsigned long long BitSize)
	{
		unsigned char* LocationInBuffer = ((unsigned char*)Buffer) + (BitOffset / 8ull);
		unsigned long long ExtractedValue = (((unsigned long long) * LocationInBuffer) >> (BitOffset % 8ull)) & ((1ull << BitSize) - 1ull);

		LocationInBuffer++;
		for (unsigned char i = 8 - (BitOffset % 8ull); i < BitSize; i += 8, LocationInBuffer++)
			ExtractedValue |= (((unsigned long long) * LocationInBuffer) & ((1ull << (BitSize - i)) - 1ull)) << ((unsigned long long)i);

		return ExtractedValue;
	}

	static void SetBitOffsetValue(void* Buffer, unsigned long long BitOffset, unsigned long long BitSize, unsigned long long Value)
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

	unsigned int GenerateDecrypt(unsigned int Phi, unsigned int Encrypt)
	{
		long long A1;
		long long A2;
		long long B1;
		long long B2;
		long long D1;
		long long D2;
		long long K;

		A1 = 1;
		A2 = 0;

		B1 = 0;
		B2 = 1;

		D1 = Phi;
		D2 = Encrypt;

		K = Phi / Encrypt;

		while (D1 != 1)
		{
			long long Temp;

			Temp = A1;
			A1 = A2 - (A1 * K);
			A2 = Temp;

			Temp = B1;
			B1 = B2 - (B1 * K);
			B2 = Temp;

			Temp = D1;
			D1 = D2 - (D1 * K);
			D2 = Temp;

			if (!D1)
				return 0;

			K = D2 / D1;
		}

		if (B1 > Phi)
			return B1 % Phi;
		else if (B1 < 0)
			return B1 + Phi;

		return B1;
	}

	bool IsCoprime(unsigned int First, unsigned int Second)
	{
		return Gcd(First, Second) == 1;
	}

	bool Gcd(unsigned int First, unsigned int Second)
	{
		unsigned int Temp;

		while (true)
		{
			Temp = First % Second;
			if (Temp == 0)
				return Second;

			First = Second;
			Second = Temp;
		}
	}

private:
	unsigned int DecryptKey = 0;
	unsigned int DecryptLength = 0;

private:
	static constexpr unsigned short Primes[] = { 
		0x0101, 0x0107, 0x010D, 0x010F, 0x0115, 0x0119, 0x011B, 0x0125,
		0x0133, 0x0137, 0x0139, 0x013D, 0x014B, 0x0151, 0x015B, 0x015D,
		0x0161, 0x0167, 0x016F, 0x0175, 0x017B, 0x017F, 0x0185, 0x018D,
		0x0191, 0x0199, 0x01A3, 0x01A5, 0x01AF, 0x01B1, 0x01B7, 0x01BB,
		0x01C1, 0x01C9, 0x01CD, 0x01CF, 0x01D3, 0x01DF, 0x01E7, 0x01EB,
		0x01F3, 0x01F7, 0x01FD, 0x0209, 0x020B, 0x021D, 0x0223, 0x022D,
		0x0233, 0x0239, 0x023B, 0x0241, 0x024B, 0x0251, 0x0257, 0x0259,
		0x025F, 0x0265, 0x0269, 0x026B, 0x0277, 0x0281, 0x0283, 0x0287,
		0x028D, 0x0293, 0x0295, 0x02A1, 0x02A5, 0x02AB, 0x02B3, 0x02BD,
		0x02C5, 0x02CF, 0x02D7, 0x02DD, 0x02E3, 0x02E7, 0x02EF, 0x02F5,
		0x02F9, 0x0301, 0x0305, 0x0313, 0x031D, 0x0329, 0x032B, 0x0335,
		0x0337, 0x033B, 0x033D, 0x0347, 0x0355, 0x0359, 0x035B, 0x035F,
		0x036D, 0x0371, 0x0373, 0x0377, 0x038B, 0x038F, 0x0397, 0x03A1,
		0x03A9, 0x03AD, 0x03B3, 0x03B9, 0x03C7, 0x03CB, 0x03D1, 0x03D7,
		0x03DF, 0x03E5, 0x03F1, 0x03F5, 0x03FB, 0x03FD, 0x0407, 0x0409,
		0x040F, 0x0419, 0x041B, 0x0425, 0x0427, 0x042D, 0x043F, 0x0443,
		0x0445, 0x0449, 0x044F, 0x0455, 0x045D, 0x0463, 0x0469, 0x047F,
		0x0481, 0x048B, 0x0493, 0x049D, 0x04A3, 0x04A9, 0x04B1, 0x04BD,
		0x04C1, 0x04C7, 0x04CD, 0x04CF, 0x04D5, 0x04E1, 0x04EB, 0x04FD,
		0x04FF, 0x0503, 0x0509, 0x050B, 0x0511, 0x0515, 0x0517, 0x051B,
		0x0527, 0x0529, 0x052F, 0x0551, 0x0557, 0x055D, 0x0565, 0x0577,
		0x0581, 0x058F, 0x0593, 0x0595, 0x0599, 0x059F, 0x05A7, 0x05AB,
		0x05AD, 0x05B3, 0x05BF, 0x05C9, 0x05CB, 0x05CF, 0x05D1, 0x05D5,
		0x05DB, 0x05E7, 0x05F3, 0x05FB, 0x0607, 0x060D, 0x0611, 0x0617,
		0x061F, 0x0623, 0x062B, 0x062F, 0x063D, 0x0641, 0x0647, 0x0649
	};
};