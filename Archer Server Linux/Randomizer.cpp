#include "Randomizer.h"
#include <time.h>
#include <stdlib.h>

#define RANDULL ((((unsigned long long)rand()) << 57) | (((unsigned long long)rand()) << 42) | (((unsigned long long)rand()) << 31) | (((unsigned long long)rand()) << 11) | ((unsigned long long)rand()))

unsigned long long Randomizer::RandomNumber()
{
	static unsigned char RandStandard = 0;
	unsigned long long Random;

	int Values[4];

	if (!RandStandard)
	{
		asm("cpuid"
			: "=a" (Values[0]),
			"=b" (Values[1]),
			"=c" (Values[2]),
			"=d" (Values[3])
			: "a" (0), "c" (0));

		if (Values[0] >= 7)
		{
			asm("cpuid"
				: "=a" (Values[0]),
				"=b" (Values[1]),
				"=c" (Values[2]),
				"=d" (Values[3])
				: "a" (7), "c" (0));

			RandStandard = ((Values[1] >> 18) & 1) * 3;
		}

		if (!RandStandard)
		{
			if (Values[0] >= 1)
			{
				asm("cpuid"
					: "=a" (Values[0]),
					"=b" (Values[1]),
					"=c" (Values[2]),
					"=d" (Values[3])
					: "a" (1), "c" (0));

				RandStandard = ((Values[0] >> 30) & 1) * 2;
			}
		}

		if (!RandStandard)
		{
			time_t Time;

			srand(time(&Time));
			RandStandard = 1;
		}
	}

	switch (RandStandard)
	{
	case 3:
	{
		asm("regen%=:"
			"rdseed %0\n\t"
			"jnc regen%="
			: "=r" (Random));
	} break;
	case 2:
	{
		asm("regen%=:"
			"rdrand %0\n\t"
			"jnc regen%="
			: "=r" (Random));
	} break;
	case 1:
	{
		Random = RANDULL;
	} break;
	}

	return Random;
}