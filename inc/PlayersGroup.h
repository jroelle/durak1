#pragma once
#include <functional>
#include "Utility.hpp"

class Deck;
class Player;
class User;

class PlayersGroup
{
public:
	PlayersGroup(size_t botsNumber = 1);
	~PlayersGroup();
	void DrawCards(Deck&, Player* start = nullptr);

	Player* Next(Player*) const;
	User* GetUser() const;
	size_t GetCount() const;

	using RemoveIfCallback = std::function<bool(const Player&)>;
	void RemoveIf(const RemoveIfCallback&);

	using ForEachCallback = std::function<bool(Player*)>;
	bool ForEach(const ForEachCallback&, Player* start = nullptr);

	using ConstForEachCallback = std::function<bool(const Player*)>;
	bool ForEach(const ConstForEachCallback&, Player* start = nullptr) const;

private:
	using PlayerLoop = utility::loop_list<Player>;
	PlayerLoop _playerLoop;
	User* _user = nullptr;
};