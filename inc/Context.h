#pragma once
#include <optional>
#include <unordered_map>
#include <list>
#include "Deck.h"
#include "PlayersGroup.h"
#include "Card.h"

class Context
{
public:
	using RoundCards = std::vector<Card>;

	Context() = delete;
	Context(size_t botsNumber = 1);

	Deck& GetDeck();
	const Deck& GetDeck() const;

	PlayersGroup& GetPlayers();
	const PlayersGroup& GetPlayers() const;

	Card::Suit GetTrumpSuit() const;

private:
	Deck _deck;
	PlayersGroup _players;
	Card::Suit _trumpSuit;
};