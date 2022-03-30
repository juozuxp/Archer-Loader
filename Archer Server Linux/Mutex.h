#pragma once
#include <string.h>
#include <stdlib.h>

class Mutex
{
public:
	constexpr Mutex()
	{
	}

	bool Lock();
	inline bool Free();
	inline volatile void WaitForLock();

private:
	bool Locked = false;
};

bool Mutex::Free()
{
	if (Locked)
	{
		Locked = false;
		return true;
	}

	return false;
}

volatile void Mutex::WaitForLock()
{
	while (!Lock());
}