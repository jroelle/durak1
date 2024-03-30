#pragma once
#include <optional>
#include <functional>
#include "Hand.h"
#include "Card.h"

class Deck;
class Context;

class Player
{
public:
	using Id = uint8_t;

	Player(Id);
	virtual ~Player() = default;

	std::optional<Card> Attack(const Context&);
	std::optional<Card> Defend(const Context&, const Card&);

	Player& DrawCards(Deck&);
	Player& DrawCards(std::vector<Card>&&);
	std::optional<Card> FindLowestTrumpCard(Card::Suit) const;
	Id GetId() const;

	Hand& GetHand() { return _hand; }
	const Hand& GetHand() const { return _hand; }

protected:
	Player() = default;

	virtual std::optional<Card> pickAttackCard(const Context&) const = 0;
	virtual std::optional<Card> pickDefendCard(const Context& , const Card&) const = 0;
	
private:
	void removeCard(const std::optional<Card>&);

private:
	Hand _hand;
	const Id _id;
};