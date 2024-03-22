#pragma once
#include "Player.h"

class Bot final : public Player
{
protected:
	std::optional<Card> pickAttackCard(const Context&) const override;
	std::optional<Card> pickDefendCard(const Context&, const Card&) const override;
};