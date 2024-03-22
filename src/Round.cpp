#include "Round.h"
#include <map>
#include "Context.h"
#include "Player.h"

namespace
{
	inline bool playersHaveAnyCards(const PlayersGroup& players, PlayersGroup::Index excludeIndex)
	{
		for (size_t i = 0; i < players.GetCount(); ++i)
		{
			if (i != excludeIndex && players.Get(i).HasAnyCards())
				return true;
		}
		return false;
	}

	inline void drawCards(Context& context)
	{
		context.GetAttacker().DrawCards(context.GetDeck());
		context.GetPlayers().ForEach([&context](Player& player)
			{
				player.DrawCards(context.GetDeck());
				return false;
			}, context.GetPlayers().Next(context.GetDefenderIndex()));
		context.GetDefender().DrawCards(context.GetDeck());
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
	auto& attacker = _context->GetAttacker();
	auto& defender = _context->GetDefender();

	roundCards.clear();
	for (size_t attackIndex = 0; attackIndex < Hand::MinCount; ++attackIndex)
	{
		auto attackCard = attacker.Attack(*_context);
		if (!attackCard)
		{
			const auto idlePlayerIndex = players.Next(_context->GetDefenderIndex());
			if (idlePlayerIndex != _context->GetAttackerIndex() && idlePlayerIndex != _context->GetDefenderIndex())
			{
				_context->GetPlayers().ForEach([this, &attackCard](Player& player)
					{
						attackCard = player.Attack(*_context);
						return attackCard.has_value();
					}, players.Next(_context->GetDefenderIndex()));
			}
		}

		if (attackCard)
		{
			roundCards.emplace_back(*attackCard);
			if (const auto defendCard = defender.Defend(*_context, *attackCard))
			{
				roundCards.emplace_back(*defendCard);
			}
			else
			{
				defender.AddCards(std::make_move_iterator(roundCards.begin()), std::make_move_iterator(roundCards.end()));
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

	const auto userIndex = players.GetUserIndex();
	if (!players.Get(userIndex).HasAnyCards())
		return nullptr;

	if (_context->GetDeck().IsEmpty() && !playersHaveAnyCards(players, userIndex))
		return nullptr;

	players.RemoveIf([](const Player& player)
		{
			return !player.HasAnyCards();
		});

	_context->ToNextAttacker();
	return std::make_unique<Round>(_context);
}


std::shared_ptr<Context> Round::GetContext() const
{
	return _context;
}