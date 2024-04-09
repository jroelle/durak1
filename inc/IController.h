#pragma once
#include <functional>
#include <optional>

class Card;
class Context;

class IController
{
public:
	using PickCardFilter = std::function<bool(const Card&)>;

	IController() = default;
	virtual ~IController() = default;
	virtual std::optional<Card> UserPickCard(const Context&, bool attacking, const PickCardFilter&) = 0;
};