#include "Context.h"
#include <set>
#include "Player.h"

Context::Context(size_t botsNumber)
	: _players(botsNumber)
{
	_trumpSuit = _deck.GetLast()->GetSuit();
	_players.DrawCards(_deck, _players.GetUser());
}

Deck& Context::GetDeck()
{
	return const_cast<Deck&>(const_cast<const Context*>(this)->GetDeck());
}

const Deck& Context::GetDeck() const
{
	return _deck;
}

PlayersGroup& Context::GetPlayers()
{
	return const_cast<PlayersGroup&>(const_cast<const Context*>(this)->GetPlayers());
}

const PlayersGroup& Context::GetPlayers() const
{
	return _players;
}

Card::Suit Context::GetTrumpSuit() const
{
	return _trumpSuit;
}