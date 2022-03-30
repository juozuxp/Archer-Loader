#pragma once

template<typename First, typename Second>
class Pair
{
public:
	constexpr Pair()
	{
	}

	constexpr Pair(const First& Element1, const Second& Element2) : Element1(Element1), Element2(Element2)
	{
	}

public:
	First Element1;
	Second Element2;
};