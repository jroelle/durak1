#pragma once
#include <stdint.h>
#include <list>

class Card
{
public:
	enum class Suit : uint8_t
	{
		Hearts,
		Diamonds,
		Clover,
		Spades
	};

	enum class Rank : uint8_t
	{
		Number6 = 6,
		Number7 = 7,
		Number8 = 8,
		Number9 = 9,
		Jack = 10,
		Queen = 11,
		King = 12,
		Ace = 13,

		Min = Number6,
		Max = Ace,
	};

	Card() = delete;
	Card(Suit suit, Rank rank)
		: _suit(suit)
		, _rank(rank)
	{}

	Suit GetSuit() const
	{
		return _suit;
	}

	Rank GetRank() const
	{
		return _rank;
	}

private:
	Suit _suit;
	Rank _rank;
};

class Cards
{
public:
	void Add(const Card&);
	void MoveTo(const Card&, Cards&);
	void Remove(const Card&);

private:
	std::list<Card> _cards;
};