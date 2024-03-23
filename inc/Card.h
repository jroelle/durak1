#pragma once
#include <stdint.h>

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
	Card(const Card&) = default;
	Card(Card&&) = default;
	Card(Suit, Rank);

	bool operator==(const Card&) const = default;
	Card& operator=(const Card&) = default;
	Card& operator=(Card&&) = default;

	Suit GetSuit() const;
	Rank GetRank() const;

	bool IsTrump(Suit) const;
	bool Beats(const Card&, Suit trumpSuit) const;

private:
	Suit _suit;
	Rank _rank;
};