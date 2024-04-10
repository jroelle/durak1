#pragma once
#include <random>

class Random final
{
public:
	using Generator = std::mt19937;

	static Generator& GetGenerator()
	{
		return _generator;
	}

	template<typename T>
	static T GetNumber(T max, T min = 0)
	{
		std::uniform_int_distribution<T> range(min, max);
		return range(_generator);
	}

private:
	inline static Generator _generator = Generator(std::random_device{}());
};