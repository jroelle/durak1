#pragma once
#include <optional>
#include "Hand.h"
#include "Card.h"

class Deck;

class Player
{
public:
	virtual ~Player() = default;

	std::optional<Card> Attack(const Player&);
	std::optional<Card> Defend(const Player&, const Card&);

	Player& DrawCards(Deck&);
	std::optional<Card> FindLowestTrumpCard(Card::Suit) const;

	template<typename T>
	Player& AddCards(T&& begin, T&& end)
	{
		_hand.AddCards(std::forward<T>(begin), std::forward<T>(end));
		return *this;
	}

protected:
	virtual std::optional<Card> pickAttackCard(const Player&) const = 0;
	virtual std::optional<Card> pickDefendCard(const Player&, const Card&) const = 0;

	Hand& getHand() { return _hand; }
	const Hand& getHand() const { return _hand; }
	
private:
	void removeCard(const std::optional<Card>&);

private:
	Hand _hand;
};