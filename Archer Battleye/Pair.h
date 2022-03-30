#pragma once

template<typename First, typename Second>
class Pair
{
public:
	constexpr Pair()
	{
	}

	constexpr Pair(const First& First, const Second& Second) : First(First), Second(Second)
	{
	}

public:
	First First;
	Second Second;
};