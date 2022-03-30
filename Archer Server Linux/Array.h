#pragma once
#include "ArrayBase.h"

template<typename T>
class Array : public ArrayBase<T>
{
public:
	inline Array()
	{
		this->TArray = 0;
	}

	inline Array(T* Buffer, unsigned int Count)
	{
		this->TArray = Buffer;
		this->ArrayNum = Count;
	}
};