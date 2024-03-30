#include "Round.h"
#include <map>
#include "Context.h"
#include "Player.h"
#include "UI.h"
#include "Event.hpp"

namespace
{
	inline bool playerHasAnyCards(const Player* player)
	{
		return !player->GetHand().IsEmpty();
	}
}

Round::Round(std::shared_ptr<Context> context, Player* attacker)
	: _context(context)
	, _attacker(attacker)
{
}

std::unique_ptr<Round> Round::Run()
{
	auto& players = _context->GetPlayers();

	auto* attacker = _attacker;
	auto* defender = players.GetDefender(attacker);
	if (!attacker || !defender)
		return nullptr;

	EventHandlers::Get().OnRoundStart(*this);

	_cards = {};
	_cards.reserve(MaxAttacksCount * 2);
	for (size_t attackIndex = 0; attackIndex < MaxAttacksCount; ++attackIndex)
	{
		std::optional<Card> attackCard;
		Player* attackerPickedCard = nullptr;
		players.ForEachAttackPlayer([this, &attackCard, &attackerPickedCard](Player* attackPlayer)
			{
				attackCard = attackPlayer->Attack(*_context);
				attackerPickedCard = attackPlayer;
				return attackCard.has_value();
			}, attacker);

		if (attackCard)
		{
			_cards.push_back(*attackCard);
			if (const auto defendCard = defender->Defend(*_context, *attackCard))
			{
				_cards.push_back(*defendCard);
			}
			else
			{
				defender->DrawCards(std::move(_cards));
				break;
			}
		}
		break;
	}

	EventHandlers::Get().OnRoundEnd(*this);

	const auto drawCards = [&](Player* player)
		{
			player->DrawCards(_context->GetDeck());
			return _context->GetDeck().IsEmpty();
		};

	players.ForEachAttackPlayer(drawCards, attacker);
	drawCards(defender);

	const auto* user = players.GetUser();
	if (!playerHasAnyCards(user))
	{
		EventHandlers::Get().OnUserWin(*user, *_context);
		return nullptr;
	}

	if (_context->GetDeck().IsEmpty() && !players.ForEachOtherPlayer(playerHasAnyCards, user))
	{
		EventHandlers::Get().OnUserLose(*players.GetUser(), *_context);
		return nullptr;
	}

	players.RemoveIf(playerHasAnyCards);
	return std::make_unique<Round>(_context, defender);
}


std::shared_ptr<Context> Round::GetContext() const
{
	return _context;
}

const Round::Cards& Round::GetCards() const
{
	return _cards;
}