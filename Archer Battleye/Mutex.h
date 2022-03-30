#pragma once
#include <intrin.h>

class Mutex
{
public:
	inline void Free()
	{
		Locked = false;
	}

	volatile bool Lock()
	{
		return !_InterlockedCompareExchange8((char*)&Locked, true, false);
	}

	inline void WaitForLock()
	{
		while (!Lock());
	}

private:
	bool Locked = false;
};