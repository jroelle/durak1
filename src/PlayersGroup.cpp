#include "PlayersGroup.h"
#include "User.h"
#include "Bot.h"

PlayersGroup::PlayersGroup(size_t botsNumber)
{
	_loop_vector.vector().reserve(botsNumber + 1);
	_loop_vector.vector().push_back(std::make_unique<User>());
	for (size_t i = 0; i < botsNumber; ++i)
		_loop_vector.vector().push_back(std::make_unique<Bot>());
}

void PlayersGroup::DrawCards(Deck& deck)
{
	ForEach([&deck](Player& player)
		{
			player.DrawCards(deck);
		});
}

PlayersGroup::LowestTrumpCards PlayersGroup::FindLowestTrumpCard(Card::Suit trumpSuit) const
{
	LowestTrumpCards lowestTrumpCards;
	for (Index i = 0; i < GetCount(); ++i)
		lowestTrumpCards[i] = Get(i).FindLowestTrumpCard(trumpSuit);
	return lowestTrumpCards;
}

PlayersGroup::Index PlayersGroup::Next(Index i) const
{
	return _loop_vector.next(std::next(_loop_vector.begin(), i));
}

Player& PlayersGroup::Get(Index i)
{
	return const_cast<Player&>(const_cast<const Player&>(Get(i)));
}

const Player& PlayersGroup::Get(Index i) const
{
	return *_loop_vector.vector().at(i);
}

size_t PlayersGroup::GetCount() const
{
	return _loop_vector.vector().size();
}

void PlayersGroup::Remove(Index i)
{
	_loop_vector.vector().erase(std::next(_loop_vector.vector().begin(), i));
}

void PlayersGroup::ForEach(const Callback& callback)
{
	for (size_t i = 0; i < GetCount(); ++i)
		callback(Get(i));
}

void PlayersGroup::ForEach(const ConstCallback& callback) const
{
	for (size_t i = 0; i < GetCount(); ++i)
		callback(Get(i));
}