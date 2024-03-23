#include "Round.h"
#include <map>
#include "Context.h"
#include "Player.h"

namespace
{
	inline bool playerHasAnyCards(const Player* player)
	{
		return player->HasAnyCards();
	}

	inline void drawCards(Context& context)
	{
		auto* attacker = context.GetAttacker();
		auto* defender = context.GetDefender();
		if (!attacker || !defender)
			return;

		context.GetPlayers().ForEachAttackPlayer([&context](Player* player)
			{
				player->DrawCards(context.GetDeck());
				return false;
			}, attacker);

		defender->DrawCards(context.GetDeck());
	}
}

Round::Round(std::shared_ptr<Context> context)
	: Observed()
	, _context(context)
{
}

std::unique_ptr<Round> Round::Run()
{
	auto& players = _context->GetPlayers();
	auto& roundCards = _context->GetRoundCards();

	auto* attacker = _context->GetAttacker();
	auto* defender = _context->GetDefender();
	if (!attacker || !defender)
		return nullptr;

	roundCards.clear();
	for (size_t attackIndex = 0; attackIndex < Hand::MinCount; ++attackIndex)
	{
		std::optional<Card> attackCard;
		players.ForEachAttackPlayer([this, &attackCard](Player* attackPlayer)
			{
				attackCard = attackPlayer->Attack(*_context);
				return attackCard.has_value();
			}, attacker);

		if (attackCard)
		{
			roundCards.emplace_back(*attackCard);
			if (const auto defendCard = defender->Defend(*_context, *attackCard))
			{
				roundCards.emplace_back(*defendCard);
			}
			else
			{
				defender->AddCards(std::make_move_iterator(roundCards.begin()), std::make_move_iterator(roundCards.end()));
				break;
			}
		}
		break;
	}

	HandleEvent([this](IObserver& observer)
		{
			observer.OnRoundUpdate(*this);
		});

	roundCards.clear();
	drawCards(*_context);

	const auto* user = players.GetUser();
	if (!user->HasAnyCards())
		return nullptr;

	if (_context->GetDeck().IsEmpty() && !players.ForEachOtherPlayer(playerHasAnyCards, user))
		return nullptr;

	players.RemoveIf(playerHasAnyCards);
	_context->ToNextPlayer();
	return std::make_unique<Round>(_context);
}


std::shared_ptr<Context> Round::GetContext() const
{
	return _context;
}