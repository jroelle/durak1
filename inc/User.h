#pragma once
#include "Player.h"

class User final : public Player
{
public:
	using Player::Player;

protected:
	std::optional<Card> pickAttackCard(const Context&, const AttackFilter&) const override;
	std::optional<Card> pickDefendCard(const Context&, const DefendFilter&) const override;
};