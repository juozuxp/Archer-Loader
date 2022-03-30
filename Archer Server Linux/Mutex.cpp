#include "Mutex.h"

bool Mutex::Lock()
{
	bool Success;

	Success = true;
	asm("xor %%eax, %%eax\n\t"
		"lock; cmpxchgb %2, %0\n\t"
		"sete %1"
		: "=m"(Locked), "=r"(Success)
		: "r"(Success)
		: "eax");

	return Success;
}