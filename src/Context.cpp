#include "Context.h"
#include <set>
#include "Player.h"

namespace
{
	inline PlayersGroup::Index findFirstPlayer(const PlayersGroup& players, Card::Suit trumpSuit)
	{
		std::optional<std::pair<PlayersGroup::Index, Card>> firstPlayer;
		for (PlayersGroup::Index i = 0; i < players.GetCount(); ++i)
		{
			const auto lowestCard = players.Get(i).FindLowestTrumpCard(trumpSuit);
			if (lowestCard && (!firstPlayer || lowestCard->GetRank() < firstPlayer->second.GetRank()))
				firstPlayer.emplace(i, *lowestCard);

			if (firstPlayer && firstPlayer->second.GetRank() == Card::Rank::Min)
				break;
		}
		return firstPlayer ? firstPlayer->first : 0;
	}
}

Context::Context(size_t botsNumber)
	: _players(botsNumber)
{
	_trumpSuit = _deck.GetLast()->GetSuit();
	_players.DrawCards(_deck);
	_attackerIndex = findFirstPlayer(_players, _trumpSuit);
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

PlayersGroup::Index Context::GetAttackerIndex() const
{
	return _attackerIndex;
}

Player& Context::GetAttacker()
{
	return const_cast<Player&>(const_cast<const Context*>(this)->GetAttacker());
}

const Player& Context::GetAttacker() const
{
	return _players.Get(_attackerIndex);
}

PlayersGroup::Index Context::GetDefenderIndex() const
{
	return _players.Next(_attackerIndex, true);
}

Player& Context::GetDefender()
{
	return const_cast<Player&>(const_cast<const Context*>(this)->GetDefender());
}

const Player& Context::GetDefender() const
{
	return _players.Get(GetDefenderIndex());
}

Card::Suit Context::GetTrumpSuit() const
{
	return _trumpSuit;
}

void Context::ToNextAttacker()
{
	_attackerIndex = _players.Next(_attackerIndex, true);
}

Context::RoundCards& Context::GetRoundCards()
{
	return const_cast<RoundCards&>(const_cast<const Context*>(this)->GetRoundCards());
}

const Context::RoundCards& Context::GetRoundCards() const
{
	return _roundCards;
}