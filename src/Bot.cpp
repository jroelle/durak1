#include "Bot.h"
#include <map>
#include <set>
#include "Card.h"
#include "Context.h"
#include "Event.hpp"

class Bot::Behavior : public EventHandler
{
public:
	Behavior(Bot&);
	virtual ~Behavior() = default;

	static std::unique_ptr<Behavior> Create(Bot&, Difficulty);

	virtual std::optional<Card> PickAttackCard(const CardFilter&) const = 0;
	virtual std::optional<Card> PickDefendCard(const CardFilter&) const = 0;

protected:
	Bot& _owner;
};

namespace
{
	struct Memory
	{
		std::map<Player::Id, std::list<Card>> playerCards;
		std::set<Card> discardPile;
	};

	class EasyBehavior : public Bot::Behavior
	{
	public:
		using Behavior::Behavior;

		std::optional<Card> PickAttackCard(const Bot::CardFilter& filter) const override
		{
			for (size_t i = 0; i < _owner.GetHand().GetCardCount(); ++i)
			{
				const Card card = _owner.GetHand().GetCard(i);
				if (!filter || filter(card))
					return card;
			}
			return std::nullopt;
		}

		std::optional<Card> PickDefendCard(const Bot::CardFilter& filter) const override
		{
			for (size_t i = 0; i < _owner.GetHand().GetCardCount(); ++i)
			{
				const Card card = _owner.GetHand().GetCard(i);
				if (!filter || filter(card))
					return card;
			}
			return std::nullopt;
		}
	};

	class MediumBehavior : public Bot::Behavior
	{
	public:
		using Behavior::Behavior;

		std::optional<Card> PickAttackCard(const Bot::CardFilter& filter) const override
		{
			// TODO
			return std::nullopt;
		}

		std::optional<Card> PickDefendCard(const Bot::CardFilter& filter) const override
		{
			// TODO
			return std::nullopt;
		}
	};

	class HardBehavior : public Bot::Behavior
	{
	public:
		using Behavior::Behavior;

		std::optional<Card> PickAttackCard(const Bot::CardFilter& filter) const override
		{
			// TODO
			return std::nullopt;
		}

		std::optional<Card> PickDefendCard(const Bot::CardFilter& filter) const override
		{
			// TODO
			return std::nullopt;
		}

		void OnPlayerShowTrumpCard(const Player& player, const Card& card) override
		{
			// TODO
		}

		void OnPlayerAttack(const Player& player, const Card& card) override
		{
			// TODO
		}

		void OnPlayerDefend(const Player& player, const Card& card) override
		{
			// TODO
		}

		void OnPlayerDrawRoundCards(const Player& player, const std::vector<Card>& cards) override
		{
			// TODO
		}

	private:
		Memory _memory;
	};
}

Bot::Behavior::Behavior(Bot& owner)
	: _owner(owner)
{
}

std::unique_ptr<Bot::Behavior> Bot::Behavior::Create(Bot& owner, Difficulty difficulty)
{
	switch (difficulty)
	{
	case Difficulty::Easy:		return std::make_unique<EasyBehavior>(owner);
	case Difficulty::Medium:	return std::make_unique<MediumBehavior>(owner);
	case Difficulty::Hard:		return std::make_unique<HardBehavior>(owner);
	}
	return nullptr;
}


Bot::Bot(Id id, Difficulty difficulty)
	: Player(id)
	, _behavior(Behavior::Create(*this, difficulty))
{
	if (_behavior)
		EventHandlers::Get().Add(_behavior.get());
}

Bot::~Bot()
{
}

std::optional<Card> Bot::pickAttackCard(const Context& context, const CardFilter& filter) const
{
	if (_behavior)
		return _behavior->PickAttackCard(filter);
	return std::nullopt;
}

std::optional<Card> Bot::pickDefendCard(const Context& context, const CardFilter& filter) const
{
	if (_behavior)
		return _behavior->PickDefendCard(filter);
	return std::nullopt;
}