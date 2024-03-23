#include "PlayersGroup.h"
#include "User.h"
#include "Bot.h"
#include "Deck.h"

PlayersGroup::PlayersGroup(size_t botsNumber)
{
	using loop = utility::loop_list<Player>;

	_user = _playerLoop.push(loop::make_holder<User>());


	_playerLoop.reserve(botsNumber + 1);
	_playerLoop.push_back(std::make_unique<User>());
	for (size_t i = 0; i < botsNumber; ++i)
		_playerLoop.push_back(std::make_unique<Bot>());
}

PlayersGroup::~PlayersGroup()
{
}

void PlayersGroup::DrawCards(Deck& deck, Player* start)
{
	ForEach([&deck](Player& player)
		{
			player.DrawCards(deck);
			return deck.IsEmpty();
		}, start);
}

PlayersGroup::Index PlayersGroup::Next(Index i) const
{
	return (i + 1) % _playerLoop.size();
}

Player& PlayersGroup::Get(Index i)
{
	return const_cast<Player&>(const_cast<const PlayersGroup*>(this)->Get(i));
}

const Player& PlayersGroup::Get(Index i) const
{
	return *_playerLoop.at(i);
}

User* PlayersGroup::GetUser() const
{
	return _user;
}

size_t PlayersGroup::GetCount() const
{
	return _playerLoop.size();
}

PlayersGroup::Index PlayersGroup::GetUserIndex() const
{
	return 0;
}

void PlayersGroup::RemoveIf(const RemoveIfCallback& removeIf)
{
	_playerLoop.erase(std::remove_if(_playerLoop.begin(), _playerLoop.end(), removeIf));
}

bool PlayersGroup::ForEach(const ForEachCallback& callback, Index start)
{
	for (size_t i = start; i < GetCount(); ++i)
	{
		if (callback(Get(i)))
			return true;
	}
	return false;
}

bool PlayersGroup::ForEach(const ConstForEachCallback& callback, Index start) const
{
	for (size_t i = start; i < GetCount(); ++i)
	{
		if (callback(Get(i)))
			return true;
	}
	return false;
}