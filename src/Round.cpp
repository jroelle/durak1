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

	HandleEvent([&](IObserver& observer)
		{
			observer.OnRoundStart(*this);
		});

	_cards = {};
	_cards.reserve(Hand::MinCount * 2);
	for (size_t attackIndex = 0; attackIndex < Hand::MinCount; ++attackIndex)
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
			_cards.emplace_back(*attackCard);
			HandleEvent([&](IObserver& observer)
				{
					observer.OnPlayerAttack(*attackerPickedCard, *attackCard);
				});

			if (const auto defendCard = defender->Defend(*_context, *attackCard))
			{
				_cards.emplace_back(*defendCard);
				HandleEvent([&](IObserver& observer)
					{
						observer.OnPlayerDefend(*attackerPickedCard, *attackCard, *defendCard);
					});
			}
			else
			{
				defender->AddCards(_cards.begin(), _cards.end());
				HandleEvent([&](IObserver& observer)
					{
						observer.OnPlayerDrawCards(*defender, *this);
					});
				break;
			}
		}
		break;
	}

	HandleEvent([&](IObserver& observer)
		{
			observer.OnRoundEnd(*this);
		});

	const auto drawCards = [&](Player* player)
		{
			player->DrawCards(_context->GetDeck());
			if (!_context->GetDeck().IsEmpty())
			{
				HandleEvent([&](IObserver& observer)
					{
						observer.OnPlayerDrawCards(*player, _context->GetDeck());
					});
			}
			return _context->GetDeck().IsEmpty();
		};

	players.ForEachAttackPlayer(drawCards, attacker);
	drawCards(defender);

	const auto* user = players.GetUser();
	if (!user->HasAnyCards())
	{
		HandleEvent([&](IObserver& observer)
			{
				observer.OnUserWin(*players.GetUser(), *_context);
			});
		return nullptr;
	}

	if (_context->GetDeck().IsEmpty() && !players.ForEachOtherPlayer(playerHasAnyCards, user))
	{
		HandleEvent([&](IObserver& observer)
			{
				observer.OnUserLose(*players.GetUser(), *_context);
			});
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