#pragma once
#include <stdint.h>
#include <memory>
#include "Player.h"

class Bot final : public Player
{
public:
	class Behavior;
	enum class Difficulty : uint8_t
	{
		Easy,
		Medium,
		Hard,

		Count,
	};

	Bot(Id, Difficulty);
	~Bot();

protected:
	std::optional<Card> pickAttackCard(const Context&, const Player& defender, const CardFilter&) const override;
	std::optional<Card> pickDefendCard(const Context&, const Player& attacker, const CardFilter&) const override;

private:
	std::unique_ptr<Behavior> _behavior;
};