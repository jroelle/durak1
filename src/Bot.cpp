#include "Bot.h"
#include "Card.h"
#include "Context.h"

std::optional<Card> Bot::pickAttackCard(const Context& context) const
{
	// TODO
	if (GetHand().IsEmpty())
		return std::nullopt;
	return GetHand().GetCard(0);
}

std::optional<Card> Bot::pickDefendCard(const Context& context, const Card& attackCard) const
{
	// TODO
	for (size_t i = 0; i < GetHand().GetCardCount(); ++i)
	{
		const Card card = GetHand().GetCard(i);
		if (card.Beats(attackCard, context.GetTrumpSuit()))
			return card;
	}
	return std::nullopt;
}