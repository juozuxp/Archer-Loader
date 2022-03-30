#pragma once

class PseudoRandom
{
public:
	constexpr PseudoRandom()
	{
	}

	inline PseudoRandom(unsigned int Seed) : Number(GenerateRandom(GenerateRandom(Seed) + 0x8B6EABC3)), SeedOffset(GenerateRandom(GenerateRandom(Seed / 2) + 0x315A87C9))
	{
	}

	inline unsigned int GetNumber()
	{
		unsigned int Result = GenerateRandom((GenerateRandom(Number) + SeedOffset) ^ 0x8468ADC7);

		Number++;
		return Result;
	}

private:
	inline unsigned int GenerateRandom(unsigned int Number)
	{
		constexpr unsigned int Prime = 4294967291;
		if (Number >= Prime)
			return Number;

		unsigned int Residue = (((unsigned long long)Number) * ((unsigned long long)Number)) % Prime;
		return (Number < (Prime / 2)) ? Residue : (Prime - Number);
	}

private:
	unsigned long Number = 0;
	unsigned long SeedOffset = 0;
};