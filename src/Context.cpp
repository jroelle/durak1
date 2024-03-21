#include "Context.h"

namespace
{
	inline void forEachOtherPlayer(const auto& players, const auto& callback)
	{
		for (size_t i = 0; i < players.GetCount(); ++i)
		{
			if (auto* player = player.Get(i))
				callback(*player);
		}
	}

	inline PlayersGroup::Index findFirstPlayer(const PlayersGroup& players)
	{
		const auto lowestTrumpCards = players.FindLowestTrumpCard();
		std::optional<std::pair<PlayersGroup::Index, Card>> firstPlayer;
		for (const auto& [index, card] : lowestTrumpCards)
		{
			if (!firstPlayer || firstPlayer->second.GetRank() > card.GetRank())
				firstPlayer = { index, card };
		}
		return firstPlayer ? firstPlayer->first : 0;
	}
}

Context::Context(size_t aiNumber)
{
	_trumpSuit = _deck.GetLast()->GetSuit();
	_players.Setup(_deck, 1);
	_attackerIndex = findFirstPlayer(_players);
}

Deck& Context::GetDeck()
{
	return const_cast<Deck&>(const_cast<const Context*>(GetDeck()));
}

const Deck& Context::GetDeck() const
{
	return _deck;
}

PlayersGroup& Context::GetPlayers()
{
	return const_cast<PlayersGroup&>(const_cast<const Context*>(GetPlayers()));
}

const PlayersGroup& Context::GetPlayers() const
{
	return _players;
}

Player& Context::GetAttacker()
{
	return const_cast<Player&>(const_cast<const Context*>(GetAttacker()));
}

const Player& Context::GetAttacker() const
{
	return _attackerIndex ? _players.Get(*_attackerIndex) : nullptr;
}

Player& Context::GetDefender()
{
	return const_cast<Player&>(const_cast<const Context*>(GetDefender()));
}

const Player& Context::GetDefender() const
{
	return _attackerIndex ? _players.Get(_players.Next(*_attackerIndex)) : nullptr;
}

void Context::ForEachOtherPlayer(const Callback& callback)
{
	forEachOtherPlayer(_players, callback);
}

void Context::ForEachOtherPlayer(const ConstCallback& callback) const
{
	forEachOtherPlayer(_players, callback);
}

void Card::Suit Context::GetTrumpSuit() const
{
	return _trumpSuit;
}

void Context::ToNextAttacker()
{
	_attackerIndex = _players.Next(_attackerIndex);
}