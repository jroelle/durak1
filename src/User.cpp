#include "User.h"
#include "Card.h"
#include "Context.h"
#include "UI.h"

std::optional<Card> User::pickAttackCard(const Context& context) const
{
	auto ui = context.GetUI();
	return ui ? ui->UserPickCard(context, *this, false) : std::nullopt;
}

std::optional<Card> User::pickDefendCard(const Context& context, const Card& attackCard) const
{
	auto ui = context.GetUI();
	return ui ? ui->UserPickCard(context, *this, true) : std::nullopt;
}