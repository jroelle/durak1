#include "PlayersGroup.h"
#include "User.h"
#include "Bot.h"
#include "Deck.h"

namespace
{
	inline bool equal(const Player* a, const Player* b)
	{
		using element = utility::loop_list<Player>::element;

		element::hash hash;
		element::equal equal;
		return hash(a) == hash(b) && equal(a, b);
	}
}

PlayersGroup::PlayersGroup(size_t botsNumber)
{
	_user = _playerLoop.push(PlayerLoop::element::make_holder<User>());
	for (size_t i = 0; i < botsNumber; ++i)
		_playerLoop.push(PlayerLoop::element::make_holder<User>());
}

PlayersGroup::~PlayersGroup()
{
}

void PlayersGroup::DrawCards(Deck& deck, Player* start)
{
	ForEach([&deck](Player* player)
		{
			player->DrawCards(deck);
			return deck.IsEmpty();
		}, start);
}

Player* PlayersGroup::Next(const Player* player) const
{
	return player ? _playerLoop.next(const_cast<Player*>(player)) : nullptr;
}

Player* PlayersGroup::GetUser() const
{
	return _user;
}

size_t PlayersGroup::GetCount() const
{
	return _playerLoop.size();
}

Player* PlayersGroup::GetDefender(const Player* attacker) const
{
	return Next(attacker);
}

void PlayersGroup::RemoveIf(const RemoveIfCallback& removeIf)
{
	std::vector<Player*> playersToRemove;
	playersToRemove.reserve(GetCount());

	_playerLoop.for_each([&removeIf, &playersToRemove](Player* player)
		{
			if (removeIf(player))
				playersToRemove.push_back(player);
		});

	for (auto* player : playersToRemove)
		_playerLoop.erase(player);
}

bool PlayersGroup::ForEach(const ForEachCallback& callback, const Player* start) const
{
	return _playerLoop.for_each(const_cast<Player*>(start), callback);
}

bool PlayersGroup::ForEachIdlePlayer(const ForEachCallback& callback, const Player* attacker) const
{
	bool result = false;
	_playerLoop.for_each(GetDefender(attacker), [&](Player* player)
		{
			result = callback(player);
			return result || equal(attacker, player);
		});
	return result;
}

bool PlayersGroup::ForEachAttackerPlayer(const ForEachCallback& callback, const Player* attacker)
{
	return ForEachOtherPlayer(callback, GetDefender(attacker), attacker);
}

bool PlayersGroup::ForEachOtherPlayer(const ForEachCallback& callback, const Player* exclude, const Player* start) const
{
	return _playerLoop.for_each(const_cast<Player*>(start), [&](Player* player)
		{
			if (equal(exclude, player))
				return false;

			return callback(player);
		});
}