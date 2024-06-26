#include "Bot.h"
#include <map>
#include <set>
#include <thread>
#include "Card.h"
#include "Context.h"
#include "Event.hpp"
#include "Random.hpp"

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
			return pickCard(filter);
		}

		std::optional<Card> PickDefendCard(const Bot::CardFilter& filter) const override
		{
			return pickCard(filter);
		}

	private:
		static Card randomPick(const std::vector<Card>& filteredCards)
		{
			const size_t index = Random::GetNumber(filteredCards.size() - 1);
			return filteredCards[index];
		}

		std::optional<Card> pickCard(const Bot::CardFilter& filter) const
		{
			std::vector<Card> filteredCards;

			const auto& hand = _owner.GetHand();
			filteredCards.reserve(hand.GetCardCount());
			for (size_t i = 0; i < hand.GetCardCount(); ++i)
			{
				const Card card = hand.GetCard(i);
				if (!filter || filter(card))
					filteredCards.push_back(card);
			}

			if (filteredCards.empty())
				return std::nullopt;

			return randomPick(filteredCards);
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
	std::this_thread::sleep_for(std::chrono::seconds(1));
	if (_behavior)
		return _behavior->PickAttackCard(filter);
	return std::nullopt;
}

std::optional<Card> Bot::pickDefendCard(const Context& context, const CardFilter& filter) const
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
	if (_behavior)
		return _behavior->PickDefendCard(filter);
	return std::nullopt;
}