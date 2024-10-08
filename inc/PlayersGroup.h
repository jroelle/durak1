#pragma once
#include <functional>
#include "Player.h"
#include "Utility.hpp"

class Deck;
struct Settings;

class PlayersGroup
{
public:
	PlayersGroup(const Settings&);
	~PlayersGroup();
	void DrawCards(Deck&, Player* start = nullptr);

	Player& Next(const Player&) const;
	Player* GetUser() const;
	size_t GetCount() const;

	Player& GetDefender(const Player& attacker) const;

	using RemoveIfCallback = std::function<bool(const Player*)>;
	void RemoveIf(const RemoveIfCallback&);

	using ForEachCallback = std::function<bool(Player*)>;
	bool ForEach(const ForEachCallback&, const Player* start = nullptr) const;
	bool ForEachIdlePlayer(const ForEachCallback&, const Player* attacker) const;
	bool ForEachAttackPlayer(const ForEachCallback&, const Player* attacker);
	bool ForEachOtherPlayer(const ForEachCallback&, const Player* exclude, const Player* start = nullptr) const;

private:
	struct Hash
	{
		size_t operator()(const Player& player) const { return std::hash<Player::Id>{}(player.GetId()); }
	};
	using PlayerLoop = utility::loop_list<Player, Hash>;

	PlayerLoop _playerLoop;
	Player* _user = nullptr;
};