#pragma once
#include "Player.h"

class Bot final : public Player
{
protected:
	std::optional<Card> pickAttackCard(const Player&) const override;
	std::optional<Card> pickDefendCard(const Player&, const Card&) const override;
};