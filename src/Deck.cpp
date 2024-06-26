#include "Deck.h"
#include <functional>
#include <algorithm>
#include "Random.hpp"

namespace
{
	inline void forEachCard(const std::function<void(const Card&)>& callback)
	{
		for (const Card::Suit suit : { Card::Suit::Hearts, Card::Suit::Diamonds, Card::Suit::Clubs, Card::Suit::Spades })
		{
			for (Card::Rank rank = Card::Rank::Min; rank <= Card::Rank::Max; rank = static_cast<Card::Rank>(static_cast<int>(rank) + 1))
			{
				callback({ suit, rank });
			}
		}
	}

	inline std::queue<Card> fillDeck()
	{
		std::deque<Card> deque;
		forEachCard([&deque](const Card& card)
			{
				deque.push_back(card);
			});

		std::shuffle(deque.begin(), deque.end(), Random::GetGenerator());
		return std::queue<Card>(std::move(deque));
	}
}

Deck::Deck()
	: _queue(fillDeck())
{
}

bool Deck::IsEmpty() const
{
	return _queue.empty();
}

std::optional<Card> Deck::GetLast() const
{
	if (IsEmpty())
		return std::nullopt;
	return _queue.back();
}

std::optional<Card> Deck::PopFirst()
{
	if (IsEmpty())
		return std::nullopt;

	Card card = _queue.front();
	_queue.pop();
	return card;
}

size_t Deck::GetCount() const
{
	return _queue.size();
}

size_t Deck::GetMaxCount()
{
	return static_cast<size_t>(Card::Suit::Count) * (static_cast<size_t>(Card::Rank::Max) - static_cast<size_t>(Card::Rank::Min) + 1);
}