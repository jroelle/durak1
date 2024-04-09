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
		Hard,

		Count,
	};

	Bot(Id, Difficulty);

protected:
	std::optional<Card> pickAttackCard(const Context&, const AttackFilter&) const override;
	std::optional<Card> pickDefendCard(const Context&, const DefendFilter&) const override;

private:
	struct Memory
	{
		// TODO
	};

	const Difficulty _difficulty;
	Memory _memory;
};