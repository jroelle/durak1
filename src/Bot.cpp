#include "Bot.h"
#include "Card.h"
#include "Context.h"
#include "Round.h"

Bot::Bot(Id id, Difficulty difficulty)
	: Player(id)
	, _difficulty(difficulty)
{
}

std::optional<Card> Bot::pickAttackCard(const Context& context, const AttackFilter& filter) const
{
	if (GetHand().IsEmpty())
		return std::nullopt;

	// TODO
	for (size_t i = 0; i < GetHand().GetCardCount(); ++i)
	{
		const Card card = GetHand().GetCard(i);
		if (!filter || filter(card))
			return card;
	}

	return std::nullopt;
}

std::optional<Card> Bot::pickDefendCard(const Context& context, const DefendFilter& filter) const
{
	// TODO
	for (size_t i = 0; i < GetHand().GetCardCount(); ++i)
	{
		const Card card = GetHand().GetCard(i);
		if (!filter || filter(card))
			return card;
	}
	return std::nullopt;
}