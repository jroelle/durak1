#pragma once
#include <stdint.h>
#include "Player.h"

class Bot final : public Player
{
public:
	enum class Difficulty : uint8_t
	{
		Easy,
		Medium,
		Difficult,
		Pro,

		Count,
	};

	Bot(Id, Difficulty);

protected:
	std::optional<Card> pickAttackCard(const Context&) const override;
	std::optional<Card> pickDefendCard(const Context&, const Card&) const override;

private:
	struct Memory
	{
		// TODO
	};

	const Difficulty _difficulty;
	Memory _memory;
};