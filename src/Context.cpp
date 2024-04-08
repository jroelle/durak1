#include "Context.h"
#include "PlayersGroup.h"
#include "Player.h"
#include "Event.hpp"

Context::Context(std::weak_ptr<UI> ui)
	: _ui(ui)
{
}

void Context::Setup(size_t botsNumber)
{
	_players = std::make_unique<PlayersGroup>(botsNumber);
	EventHandlers::Get().OnPlayersCreated(*_players);
	_trumpSuit = _deck.GetLast()->GetSuit();
	_players->DrawCards(_deck, _players->GetUser());
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
	return *_players;
}

Card::Suit Context::GetTrumpSuit() const
{
	return _trumpSuit;
}

std::shared_ptr<UI> Context::GetUI() const
{
	return _ui.lock();
}