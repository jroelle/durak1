#pragma once
#include <functional>
#include <memory>
#include <map>
#include <optional>
#include "Utility.hpp"

class Deck;
class Player;

class PlayersGroup
{
public:
	using Index = size_t;

	PlayersGroup(size_t botsNumber = 1);
	void DrawCards(Deck&);

	using LowestTrumpCards = std::map<Index, std::optional<Card>>;
	LowestTrumpCards FindLowestTrumpCard(Card::Suit) const;

	Index Next(Index) const;
	Player& Get(Index);
	const Player& Get(Index) const;
	size_t GetCount() const;
	void Remove(Index);

	using Callback = std::function<void(Player&)>;
	void ForEach(const Callback&);

	using ConstCallback = std::function<void(const Player&)>;
	void ForEach(const ConstCallback&) const;

private:
	using Value = std::unique_ptr<Player>;
	using LoopVector = utility::loop_vector<Value>;

	LoopVector _loop_vector;
};