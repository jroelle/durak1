#include "Round.h"
#include <map>
#include "Context.h"
#include "Player.h"
#include "UI.h"

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
	auto ui = _context->GetUI();
	auto& players = _context->GetPlayers();

	auto* attacker = _attacker;
	auto* defender = players.GetDefender(attacker);
	if (!attacker || !defender)
		return nullptr;

	if (ui)
		ui->OnRoundStart(*this);

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
			if (ui)
				ui->OnPlayerAttack(*attackerPickedCard, *attackCard);

			if (const auto defendCard = defender->Defend(*_context, *attackCard))
			{
				_cards.push_back(*defendCard);
				if (ui)
					ui->OnPlayerDefend(*attackerPickedCard, *attackCard, *defendCard);
			}
			else
			{
				defender->AddCards(_cards.begin(), _cards.end());
				if (ui)
					ui->OnPlayerDrawCards(*defender, *this);
				_cards.clear();
				break;
			}
		}
		break;
	}

	if (ui)
		ui->OnRoundEnd(*this);

	const auto drawCards = [&](Player* player)
		{
			player->DrawCards(_context->GetDeck());
			if (!_context->GetDeck().IsEmpty() && ui)
				ui->OnPlayerDrawCards(*player, _context->GetDeck());
			return _context->GetDeck().IsEmpty();
		};

	players.ForEachAttackPlayer(drawCards, attacker);
	drawCards(defender);

	const auto* user = players.GetUser();
	if (!user->HasAnyCards())
	{
		if (ui)
			ui->OnUserWin(*players.GetUser(), *_context);
		return nullptr;
	}

	if (_context->GetDeck().IsEmpty() && !players.ForEachOtherPlayer(playerHasAnyCards, user))
	{
		if (ui)
			ui->OnUserLose(*players.GetUser(), *_context);
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