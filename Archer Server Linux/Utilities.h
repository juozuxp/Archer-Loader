#pragma once

#define GetStructOffset(Struct, Element) ((unsigned long long)&(((Struct*)0)->Element))

class Utilities
{
public:
	static void BufferToString(void* Buffer, unsigned int BufferSize, char* StringBuffer, bool Upper);
};