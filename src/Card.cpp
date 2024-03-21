#include "Card.h"

Card::Card(Suit suit, Rank rank)
	: _suit(suit)
	, _rank(rank)
{
}

Card::Suit Card::GetSuit() const
{
	return _suit;
}

Card::Rank Card::GetRank() const
{
	return _rank;
}

bool Card::IsTrump(Suit trumpSuit) const
{
	return GetSuit() == trumpSuit;
}

bool Card::Beats(const Card& other, Suit trumpSuit) const
{
	if (IsTrump(trumpSuit) == other.IsTrump(trumpSuit))
		return GetSuit() == other.GetSuit() && GetRank() > other.GetRank();
	else if (IsTrump(trumpSuit) && !other.IsTrump(trumpSuit))
		return true;
	else
		return false;
}