#pragma once
#include <memory>
#include <vector>
#include "Hand.h"

class Context;
class Player;

class Round
{
public:
	using Cards = std::vector<Card>;
	static constexpr size_t MaxAttacksCount = Hand::MinCount;

	Round() = delete;
	Round(const Round&) = delete;

	Round(Player& attacker, Player& defender);
	Round(Round&&) = default;

	std::unique_ptr<Round> Run(Context&);
	const Cards& GetCards() const;
	Player& GetAttacker() const;
	Player& GetDefender() const;

private:
	Player& _attacker;
	Player& _defender;
	Cards _cards;
};