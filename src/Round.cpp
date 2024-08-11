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

	bool defenderLost = false;
	_cards.reserve(MaxAttacksCount * 2);
	for (size_t attackIndex = 0; attackIndex < std::min(MaxAttacksCount, _defender.GetHand().GetCardCount()); ++attackIndex)
	{
		std::optional<Card> attackCard;
		players.ForEachAttackPlayer([&](Player* attackPlayer)
			{
				attackCard = attackPlayer->Attack(context, _defender, [&](const Card& card) -> bool
					{
						return _cards.empty()
							|| std::find_if(_cards.cbegin(), _cards.cend(), [&card](const Card& roundCard) { return roundCard.GetRank() == card.GetRank(); }) != _cards.cend();
					});
				return attackCard.has_value();
			}, &_attacker);

		if (attackCard)
		{
			_cards.push_back(*attackCard);
			if (const auto defendCard = _defender.Defend(context, _attacker, [&](const Card& card) -> bool
				{
					return card.Beats(*attackCard, context.GetTrumpSuit());
				}))
			{
				_cards.push_back(*defendCard);
			}
			else
			{
				_defender.DrawCards(_cards);
				defenderLost = true;
				break;
			}
		}
		else
		{
			break;
		}
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

	auto& nextAttacker = defenderLost ? players.Next(_defender) : _defender;
	return std::make_unique<Round>(nextAttacker, players.GetDefender(nextAttacker));
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