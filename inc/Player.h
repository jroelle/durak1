#pragma once
#include <optional>
#include "Hand.h"
#include "Card.h"

class Deck;
class Context;

class Player
{
public:
	virtual ~Player() = default;

	std::optional<Card> Attack(const Context&);
	std::optional<Card> Defend(const Context&, const Card&);

	Player& DrawCards(Deck&);
	std::optional<Card> FindLowestTrumpCard(Card::Suit) const;
	size_t GetCardCount() const;
	bool HasAnyCards() const;

	template<typename T>
	Player& AddCards(T&& begin, T&& end)
	{
		_hand.AddCards(std::forward<T>(begin), std::forward<T>(end));
		return *this;
	}

protected:
	Player() = default;

	virtual std::optional<Card> pickAttackCard(const Context&) const = 0;
	virtual std::optional<Card> pickDefendCard(const Context& , const Card&) const = 0;

	Hand& getHand() { return _hand; }
	const Hand& getHand() const { return _hand; }
	
private:
	void removeCard(const std::optional<Card>&);

private:
	Hand _hand;
};