#pragma once
#include <memory>
#include <forward_list>
#include <functional>
#include "Card.h"

class Deck;
class Player;

class PlayersGroup
{
public:
	using Index = size_t;

	PlayersGroup(size_t botsNumber = 1);
	~PlayersGroup();
	void DrawCards(Deck&, Index start = 0);

	Index Next(Index) const;
	Player& Get(Index);
	const Player& Get(Index) const;
	size_t GetCount() const;
	Index GetUserIndex() const;

	using RemoveIfCallback = std::function<bool(const Player&)>;
	void RemoveIf(const RemoveIfCallback&);

	using ForEachCallback = std::function<bool(Player&)>;
	bool ForEach(const ForEachCallback&, Index start = 0);

	using ConstForEachCallback = std::function<bool(const Player&)>;
	bool ForEach(const ConstForEachCallback&, Index start = 0) const;

private:
	std::forward_list<std::unique_ptr<Player>> _playerLoop;
};