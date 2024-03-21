#pragma once
#include <functional>
#include <optional>
#include "Deck.h"
#include "PlayersGroup.h"
#include "Card.h"

class Context
{
public:
	Context() = delete;
	Context(size_t aiNumber = 1);

	Deck& GetDeck();
	const Deck& GetDeck() const;

	PlayersGroup& GetPlayers();
	const PlayersGroup& GetPlayers() const;

	Player& GetAttacker();
	const Player& GetAttacker() const;

	Player& GetDefender();
	const Player& GetDefender() const;

	using Callback = std::function<void(Player&)>;
	void ForEachOtherPlayer(const Callback&);

	using ConstCallback = std::function<void(const Player&)>;
	void ForEachOtherPlayer(const ConstCallback&) const;

	void Card::Suit GetTrumpSuit() const;
	void ToNextAttacker();

private:
	Deck _deck;
	PlayersGroup _players;
	PlayersGroup::Index _attackerIndex;
	Card::Suit _trumpSuit;
};