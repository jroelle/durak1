#include "Round.h"
#include "Context.h"

Round::Round(std::shared_ptr<Context> context)
	: _context(context)
{
}

std::unique_ptr<Round> Round::Run()
{
	auto& attacker = _context->GetAttacker();
	auto& defender = _context->GetDefender();

	while (true)
	{
		const auto attackCard = attacker.Attack(_defender);
		if (attackCard)
		{
			const auto defendCard = defender.Defend(attacker, *attackCard);
			_cards[*attackCard] = defendCard;
			if (!defendCard)
			{

				break;
			}
		}
		break;
	}

	_context->ToNextAttacker();
	const auto nextRound = std::make_unique<Round>(_context);
}
