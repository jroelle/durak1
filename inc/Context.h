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

	Player* GetAttacker();
	const Player* GetAttacker() const;

	Player* GetDefender();
	const Player* GetDefender() const;

	Card::Suit GetTrumpSuit() const;
	void ToNextPlayer();

	RoundCards& GetRoundCards();
	const RoundCards& GetRoundCards() const;

private:
	Deck _deck;
	PlayersGroup _players;
	Player* _attacker = nullptr;
	Card::Suit _trumpSuit;
	RoundCards _roundCards;
};