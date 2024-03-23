#pragma once
#include <functional>
#include "Player.h"
#include "Utility.hpp"

class Deck;

class PlayersGroup
{
public:
	PlayersGroup(size_t botsNumber = 1);
	~PlayersGroup();
	void DrawCards(Deck&, Player* start = nullptr);

	Player* Next(const Player*) const;
	Player* GetUser() const;
	size_t GetCount() const;

	Player* GetDefender(const Player* attacker) const;

	using RemoveIfCallback = std::function<bool(const Player*)>;
	void RemoveIf(const RemoveIfCallback&);

	using ForEachCallback = std::function<bool(Player*)>;
	bool ForEach(const ForEachCallback&, const Player* start = nullptr) const;
	bool ForEachIdlePlayer(const ForEachCallback&, const Player* attacker) const;
	bool ForEachAttackPlayer(const ForEachCallback&, const Player* attacker);
	bool ForEachOtherPlayer(const ForEachCallback&, const Player* exclude, const Player* start = nullptr) const;

private:
	using PlayerLoop = utility::loop_list<Player>;

	PlayerLoop _playerLoop;
	Player* _user = nullptr;
};