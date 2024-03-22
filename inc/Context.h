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

	PlayersGroup::Index GetAttackerIndex() const;
	Player& GetAttacker();
	const Player& GetAttacker() const;

	PlayersGroup::Index GetDefenderIndex() const;
	Player& GetDefender();
	const Player& GetDefender() const;

	Card::Suit GetTrumpSuit() const;
	void ToNextAttacker();

	RoundCards& GetRoundCards();
	const RoundCards& GetRoundCards() const;

private:
	Deck _deck;
	PlayersGroup _players;
	PlayersGroup::Index _attackerIndex;
	Card::Suit _trumpSuit;
	RoundCards _roundCards;
};