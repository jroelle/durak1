#pragma once
#include <stdint.h>

class Card
{
public:
	enum class Suit : uint8_t
	{
		Hearts,
		Diamonds,
		Clubs,
		Spades,

		Count
	};

	enum class Rank : uint8_t
	{
		Number6 = 6,
		Number7 = 7,
		Number8 = 8,
		Number9 = 9,
		Number10 = 10,
		Jack = 11,
		Queen = 12,
		King = 13,
		Ace = 14,

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