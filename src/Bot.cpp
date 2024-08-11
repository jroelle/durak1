#include "Bot.h"
#include <map>
#include <set>
#include <thread>
#include "Card.h"
#include "Context.h"
#include "Event.hpp"
#include "Random.hpp"
#include "Round.h"

class Bot::Behavior
{
public:
	Behavior(Bot&);
	virtual ~Behavior() = default;

	static std::unique_ptr<Behavior> Create(Bot&, Difficulty);

	virtual std::optional<Card> PickAttackCard(const Player& defender, const CardFilter&) const = 0;
	virtual std::optional<Card> PickDefendCard(const Player& attacker, const CardFilter&) const = 0;

protected:
	Bot& _owner;
};

namespace
{
	class Memory final : public AutoEventHandler
	{
	public:
		const std::set<Card>* GetPlayerCards(Player::Id id) const
		{
			auto iter = _playerCards.find(id);
			return iter != _playerCards.end() ? &iter->second : nullptr;
		}

		const std::set<Card>& GetDiscardPile() const
		{
			return _discardPile;
		}

	private:
		void OnPlayerShowTrumpCard(const Player& player, const Card& card) override
		{
			_playerCards[player.GetId()].insert(card);
		}

		void OnPlayerAttack(const Player& player, const Card& card) override
		{
			_playerCards[player.GetId()].insert(card);
		}

		void OnPlayerDefend(const Player& player, const Card& card) override
		{
			_playerCards[player.GetId()].insert(card);
		}

		void OnPlayerDrawRoundCards(const Player& player, const std::vector<Card>& cards) override
		{
			_playerCards[player.GetId()].insert(cards.begin(), cards.end());

			for (auto& [playerId, playerCards] : _playerCards)
			{
				if (playerId != player.GetId())
				{
					for (const auto& card : cards)
						playerCards.erase(card);
				}
			}
		}

		void OnRoundEnd(const Round& round) override
		{
			const auto& roundCards = round.GetCards();
			_discardPile.insert(roundCards.begin(), roundCards.end());

			for (auto& [playerId, playerCards] : _playerCards)
			{
				for (const auto& card : roundCards)
					playerCards.erase(card);
			}
		}

	private:
		std::map<Player::Id, std::set<Card>> _playerCards;
		std::set<Card> _discardPile;
	};

	class EasyBehavior : public Bot::Behavior
	{
	public:
		using Behavior::Behavior;

		std::optional<Card> PickAttackCard(const Player& defender, const Bot::CardFilter& filter) const override
		{
			return pickCard(filter);
		}

		std::optional<Card> PickDefendCard(const Player& attacker, const Bot::CardFilter& filter) const override
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

		std::optional<Card> PickAttackCard(const Player& defender, const Bot::CardFilter& filter) const override
		{
			// TODO
			return std::nullopt;
		}

		std::optional<Card> PickDefendCard(const Player& attacker, const Bot::CardFilter& filter) const override
		{
			// TODO
			return std::nullopt;
		}
	};

	class HardBehavior : public Bot::Behavior
	{
	public:
		using Behavior::Behavior;

		std::optional<Card> PickAttackCard(const Player& defender, const Bot::CardFilter& filter) const override
		{
			if (const auto* defenderCards = _memory.GetPlayerCards(defender.GetId()))
			{
				// TODO
			}
			return std::nullopt;
		}

		std::optional<Card> PickDefendCard(const Player& attacker, const Bot::CardFilter& filter) const override
		{
			if (const auto* attackerCards = _memory.GetPlayerCards(attacker.GetId()))
			{
				// TODO
			}
			return std::nullopt;
		}

	private:
		static inline Memory _memory;
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
}

Bot::~Bot()
{
}

std::optional<Card> Bot::pickAttackCard(const Context& context, const Player& defender, const CardFilter& filter) const
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
	if (_behavior)
		return _behavior->PickAttackCard(defender, filter);
	return std::nullopt;
}

std::optional<Card> Bot::pickDefendCard(const Context& context, const Player& attacker, const CardFilter& filter) const
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
	if (_behavior)
		return _behavior->PickDefendCard(attacker, filter);
	return std::nullopt;
}