#pragma once
#include <stdint.h>

struct Settings
{
	enum class Difficulty : uint8_t
	{
		Easy,
		Medium,
		Hard,

		Count,
	};

	Difficulty difficulty = Difficulty::Medium;
	size_t botsNumber = 1;
};