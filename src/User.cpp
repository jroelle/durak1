#include "User.h"
#include "Card.h"
#include "Context.h"
#include "UI.h"

std::optional<Card> User::pickAttackCard(const Context& context, const CardFilter& filter) const
{
	auto ui = context.GetUI();
	return ui ? ui->UserPickCard(context, true, filter) : std::nullopt;
}

std::optional<Card> User::pickDefendCard(const Context& context, const CardFilter& filter) const
{
	auto ui = context.GetUI();
	return ui ? ui->UserPickCard(context, false, filter) : std::nullopt;
}