#include "User.h"
#include "Card.h"
#include "Context.h"
#include "UI.h"

std::optional<Card> User::pickAttackCard(const Context& context) const
{
	return context.GetUI().UserPickCard(*this);
}

std::optional<Card> User::pickDefendCard(const Context& context, const Card& attackCard) const
{
	return context.GetUI().UserPickCard(*this);
}