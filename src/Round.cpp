#include "Round.h"
#include <map>
#include "Context.h"
#include "Player.h"
#include "UI.h"
#include "Event.hpp"
#include "PlayersGroup.h"

namespace
{
	inline bool hasAnyCards(const Player* player)
	{
		return !player->GetHand().IsEmpty();
	}

	inline bool hasNoCards(const Player* player)
	{
		return !hasAnyCards(player);
	}
}

Round::Round(Player& attacker, Player& defender)
	: _attacker(attacker)
	, _defender(defender)
{
}

std::unique_ptr<Round> Round::Run(Context& context)
{
	_cards = {};
	auto& players = context.GetPlayers();
	EventHandlers::Get().OnRoundStart(*this);

	_cards.reserve(MaxAttacksCount * 2);
	for (size_t attackIndex = 0; attackIndex < MaxAttacksCount; ++attackIndex)
	{
		std::optional<Card> attackCard;
		Player* attackerPickedCard = nullptr;
		players.ForEachAttackPlayer([&](Player* attackPlayer)
			{
				attackCard = attackPlayer->Attack(context);
				attackerPickedCard = attackPlayer;
				return attackCard.has_value();
			}, &_attacker);

		if (attackCard)
		{
			_cards.push_back(*attackCard);
			if (const auto defendCard = _defender.Defend(context, *attackCard))
			{
				_cards.push_back(*defendCard);
			}
			else
			{
				_defender.DrawCards(std::move(_cards));
				break;
			}
		}
		break;
	}

	EventHandlers::Get().OnRoundEnd(*this);

	const auto drawCards = [&](Player* player)
		{
			player->DrawCards(context.GetDeck());
			return context.GetDeck().IsEmpty();
		};

	players.ForEachAttackPlayer(drawCards, &_attacker);
	drawCards(&_defender);

	const auto* user = players.GetUser();
	if (hasNoCards(user))
	{
		EventHandlers::Get().OnUserWin(*user);
		return nullptr;
	}

	if (context.GetDeck().IsEmpty() && players.ForEachOtherPlayer(hasNoCards, user))
	{
		EventHandlers::Get().OnUserLose(*players.GetUser());
		return nullptr;
	}

	players.RemoveIf(hasNoCards);
	return std::make_unique<Round>(_defender, players.GetDefender(_defender));
}

const Round::Cards& Round::GetCards() const
{
	return _cards;
}

Player& Round::GetAttacker() const
{
	return _attacker;
}

Player& Round::GetDefender() const
{
	return _defender;
}