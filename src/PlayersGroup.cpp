#include "PlayersGroup.h"
#include "Deck.h"
#include "User.h"
#include "Bot.h"

PlayersGroup::PlayersGroup(size_t botsNumber)
{
	Player::Id id = 0;
	_user = _playerLoop.push_back(PlayerLoop::element::make_holder<User>(id++));
	for (size_t i = 0; i < botsNumber; ++i)
		_playerLoop.push_back(PlayerLoop::element::make_holder<Bot>(id++, Bot::Difficulty::Easy));
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

Player& PlayersGroup::Next(const Player& player) const
{
	return *_playerLoop.next(&player);
}

Player* PlayersGroup::GetUser() const
{
	return _user;
}

size_t PlayersGroup::GetCount() const
{
	return _playerLoop.size();
}

Player& PlayersGroup::GetDefender(const Player& attacker) const
{
	return Next(attacker);
}

void PlayersGroup::RemoveIf(const RemoveIfCallback& removeIf)
{
	std::vector<const Player*> playersToRemove;
	playersToRemove.reserve(GetCount());

	_playerLoop.for_each([&removeIf, &playersToRemove](const Player* player)
		{
			if (removeIf(player))
				playersToRemove.push_back(player);
			return false;
		});

	for (const auto* player : playersToRemove)
		_playerLoop.erase(player);
}

bool PlayersGroup::ForEach(const ForEachCallback& callback, const Player* start) const
{
	return _playerLoop.for_each(start ? start : GetUser(), callback);
}

bool PlayersGroup::ForEachIdlePlayer(const ForEachCallback& callback, const Player* attacker) const
{
	if (!attacker)
		return false;

	bool result = false;
	_playerLoop.for_each(&Next(GetDefender(*attacker)), [&](Player* player)
		{
			result = callback(player);
			return result || attacker->GetId() == player->GetId();
		});
	return result;
}

bool PlayersGroup::ForEachAttackPlayer(const ForEachCallback& callback, const Player* attacker)
{
	return attacker && ForEachOtherPlayer(callback, &GetDefender(*attacker), attacker);
}

bool PlayersGroup::ForEachOtherPlayer(const ForEachCallback& callback, const Player* exclude, const Player* start) const
{
	if (!start)
		start = GetUser();

	if (exclude)
	{
		return _playerLoop.for_each(start, [&](Player* player)
			{
				if (exclude->GetId() == player->GetId())
					return false;

				return callback(player);
			});
	}
	else
	{
		return ForEach(callback);
	}
}