#include "Hand.h"

bool Hand::IsEmpty() const
{
	return _deque.empty();
}

size_t Hand::GetCardCount() const
{
	return _deque.size();
}

Card Hand::GetCard(size_t i) const
{
	return _deque.at(i);
}

Hand& Hand::AddCard(const Card& card)
{
	_deque.push_back(card);
	return *this;
}

Hand& Hand::RemoveCard(const Card& card)
{
	auto iter = std::find(_deque.begin(), _deque.end(), card);
	if (iter != _deque.end())
		_deque.erase(iter);
	return *this;
}

bool Hand::ForEachCard(const Callback& callback) const
{
	for (size_t i = 0; i < GetCardCount(); ++i)
	{
		if (callback(_deque[i]))
			return true;
	}
	return false;
}