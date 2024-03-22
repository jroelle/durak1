#pragma once
#include <memory>
#include <map>
#include <vector>
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

	Index Next(Index, bool onlyWithCards) const;
	Player& Get(Index);
	const Player& Get(Index) const;
	size_t GetCount() const;
	Index GetUserIndex() const;

	using Callback = std::function<bool(Player&)>;
	bool ForEach(const Callback&, bool onlyWithCards, Index start = 0);

	using ConstCallback = std::function<bool(const Player&)>;
	bool ForEach(const ConstCallback&, bool onlyWithCards, Index start = 0) const;

private:
	Index getNext(Index) const;

private:
	std::vector<std::unique_ptr<Player>> _playerLoop;
};