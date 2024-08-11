#include "Card.h"

Card::Card(Suit suit, Rank rank)
	: _suit(suit)
	, _rank(rank)
{
}

bool Card::operator<(const Card& card) const
{
	if (_suit == card._suit)
		return _rank < card._rank;
	else
		return _suit < card._suit;
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