#include "Randomizer.h"
#include <Imports.h>
#include <intrin.h>

#define RANDULL ((((unsigned long long)rand()) << 57) | (((unsigned long long)rand()) << 42) | (((unsigned long long)rand()) << 31) | (((unsigned long long)rand()) << 11) | ((unsigned long long)rand()))

unsigned long long Randomizer::RandomNumber()
{
	static unsigned char RandStandard = 0;
	unsigned long long Random;

	int Values[4];

	if (!RandStandard)
	{
		__cpuid(Values, 0);
		if (Values[0] >= 7)
		{
			__cpuid(Values, 7);
			RandStandard = ((Values[1] >> 18) & 1) * 3;
		}

		if (!RandStandard)
		{
			if (Values[0] >= 1)
			{
				__cpuid(Values, 1);
				RandStandard = ((Values[0] >> 30) & 1) * 2;
			}
		}

		if (!RandStandard)
		{
			srand(GetTickCount());
			RandStandard = 1;
		}
	}

	switch (RandStandard)
	{
	case 3:
	{
		while (!_rdseed64_step(&Random));
	} break;
	case 2:
	{
		while (!_rdrand64_step(&Random));
	} break;
	case 1:
	{
		Random = RANDULL;
	} break;
	}

	return Random;
}