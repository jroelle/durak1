#include "PlayersGroup.h"
#include "User.h"
#include "Bot.h"
#include "Deck.h"

PlayersGroup::PlayersGroup(size_t botsNumber)
{
	_playerLoop.reserve(botsNumber + 1);
	_playerLoop.push_back(std::make_unique<User>());
	for (size_t i = 0; i < botsNumber; ++i)
		_playerLoop.push_back(std::make_unique<Bot>());
}

PlayersGroup::~PlayersGroup()
{
}

void PlayersGroup::DrawCards(Deck& deck, Index start)
{
	ForEach([&deck](Player& player)
		{
			player.DrawCards(deck);
			return deck.IsEmpty();
		}, false, start);
}

PlayersGroup::Index PlayersGroup::Next(Index i, bool onlyWithCards) const
{
	if (!onlyWithCards)
		return getNext(i);

	size_t next = i;
	for (size_t step = 0; step < _playerLoop.size(); ++step)
	{
		next = getNext(next);
		if (next == i)
			break;

		if (Get(next).HasAnyCards())
			return next;
	}
	return -1;
}

Player& PlayersGroup::Get(Index i)
{
	return const_cast<Player&>(const_cast<const PlayersGroup*>(this)->Get(i));
}

const Player& PlayersGroup::Get(Index i) const
{
	return *_playerLoop.at(i);
}

size_t PlayersGroup::GetCount() const
{
	return _playerLoop.size();
}

PlayersGroup::Index PlayersGroup::GetUserIndex() const
{
	return 0;
}

bool PlayersGroup::ForEach(const Callback& callback, bool onlyWithCards, Index start)
{
	for (size_t i = start; i < GetCount(); i = Next(i, onlyWithCards))
	{
		if (callback(Get(i)))
			return true;
	}
	return false;
}

bool PlayersGroup::ForEach(const ConstCallback& callback, bool onlyWithCards, Index start) const
{
	for (size_t i = start; i < GetCount(); i = Next(i, onlyWithCards))
	{
		if (callback(Get(i)))
			return true;
	}
	return false;
}

PlayersGroup::Index PlayersGroup::getNext(Index i) const
{
	return (i + 1) % _playerLoop.size();
}