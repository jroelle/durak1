#include "Bot.h"
#include "Card.h"

std::optional<Card> Bot::pickAttackCard(const Player& opponent) const
{
	// TODO
	if (getHand().IsEmpty())
		return std::nullopt;
	return getHand().GetCard(0);
}

std::optional<Card> Bot::pickDefendCard(const Player& opponent, const Card& attackCard) const
{
	// TODO
	if (size_t i = 0; i < getHand().GetCardCount(); ++i)
	{
		const Card card = getHand().GetCard(i);
		if (card.Beats(attackCard))
			return card;
	}
	return std::nullopt;
}