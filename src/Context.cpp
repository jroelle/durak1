#include "Context.h"
#include <set>
#include "Player.h"

namespace
{
	inline Player* findFirstPlayer(const PlayersGroup& players, Card::Suit trumpSuit)
	{
		std::optional<std::pair<Player*, Card>> firstPlayer;

		players.ForEach([&firstPlayer, trumpSuit](Player* player)
			{
				const auto card = player->FindLowestTrumpCard(trumpSuit);
				if (card && (!firstPlayer || card->GetRank() < firstPlayer->second.GetRank()))
					firstPlayer.emplace(player, *card);

				return firstPlayer && firstPlayer->second.GetRank() == Card::Rank::Min;
			});

		return firstPlayer ? firstPlayer->first : players.GetUser();
	}
}

Context::Context(size_t botsNumber)
	: _players(botsNumber)
{
	_trumpSuit = _deck.GetLast()->GetSuit();
	_players.DrawCards(_deck);
	_attacker = findFirstPlayer(_players, _trumpSuit);
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

Player* Context::GetAttacker()
{
	return const_cast<Player*>(const_cast<const Context*>(this)->GetAttacker());
}

const Player* Context::GetAttacker() const
{
	return _attacker;
}

Player* Context::GetDefender()
{
	return const_cast<Player*>(const_cast<const Context*>(this)->GetDefender());
}

const Player* Context::GetDefender() const
{
	return _players.Next(_attacker);
}

Card::Suit Context::GetTrumpSuit() const
{
	return _trumpSuit;
}

void Context::ToNextPlayer()
{
	_attacker = _players.GetDefender(_attacker);
}

Context::RoundCards& Context::GetRoundCards()
{
	return const_cast<RoundCards&>(const_cast<const Context*>(this)->GetRoundCards());
}

const Context::RoundCards& Context::GetRoundCards() const
{
	return _roundCards;
}