#pragma once
#include <deque>
#include <functional>
#include "Card.h"

class Hand
{
public:
	static constexpr size_t MinCount = 6;

	Hand() = default;

	bool IsEmpty() const;
	size_t GetCardCount() const;
	Card GetCard(size_t) const;
	Hand& AddCard(const Card&);
	Hand& RemoveCard(const Card&);

	using Callback = std::function<bool(const Card&)>;
	bool ForEachCard(const Callback&) const;

	template<typename T>
	Hand& AddCards(T&& begin, T&& end)
	{
		_deque.insert(_deque.end(), std::forward<T>(begin), std::forward<T>(end));
		return *this;
	}

private:
	std::deque<Card> _deque;
};