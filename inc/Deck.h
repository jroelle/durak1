#pragma once
#include "Card.h"
#include <queue>
#include <optional>

class Deck
{
public:
	Deck();

	bool IsEmpty() const;
	std::optional<Card> GetLast() const;
	std::optional<Card> PopFirst();
	size_t GetCount() const;
	static size_t GetMaxCount();

private:
	std::queue<Card> _queue;
};