#include "Bot.h"
#include <map>
#include <set>
#include <thread>
#include <array>
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

	std::optional<Card> PickAttackCard(const Context& context, const Player& defender, const CardFilter& filter) const
	{
		return pickAttackCard(context, defender, getFilteredCards(filter));
	}

	std::optional<Card> PickDefendCard(const Context& context, const Player& attacker, const CardFilter& filter) const
	{
		return pickDefendCard(context, attacker, getFilteredCards(filter));
	}

protected:
	virtual std::optional<Card> pickAttackCard(const Context&, const Player& defender, std::vector<Card>&& filteredCards) const = 0;
	virtual std::optional<Card> pickDefendCard(const Context&, const Player& attacker, std::vector<Card>&& filteredCards) const = 0;

private:
	std::vector<Card> getFilteredCards(const CardFilter& filter) const
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
		return filteredCards;
	}

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

	protected:
		std::optional<Card> pickAttackCard(const Context& context, const Player& defender, std::vector<Card>&& filteredCards) const override
		{
			return randomPick(filteredCards);
		}

		std::optional<Card> pickDefendCard(const Context& context, const Player& attacker, std::vector<Card>&& filteredCards) const override
		{
			return randomPick(filteredCards);
		}

	private:
		static std::optional<Card> randomPick(const std::vector<Card>& filteredCards)
		{
			if (filteredCards.empty())
				return std::nullopt;

			const size_t index = Random::GetNumber(filteredCards.size() - 1);
			return filteredCards[index];
		}
	};

	class MediumBehavior : public EasyBehavior
	{
	public:
		using EasyBehavior::EasyBehavior;

	protected:
		std::optional<Card> pickAttackCard(const Context& context, const Player& defender, std::vector<Card>&& filteredCards) const override
		{
			const auto& deck = context.GetDeck();
			const double pickTrumpChance = getDiscardDeckRatio(deck);
			return pickCard(context, pickTrumpChance, std::move(filteredCards));
		}

		std::optional<Card> pickDefendCard(const Context& context, const Player& attacker, std::vector<Card>&& filteredCards) const override
		{
			const auto& deck = context.GetDeck();
			const double pickTrumpChance = deck.GetCount() <= 10 ? 1. : getDiscardDeckRatio(deck) * 0.8;
			return pickCard(context, pickTrumpChance, std::move(filteredCards));
		}

		static void sort(std::vector<Card>& cards, Card::Suit trumpSuit)
		{
			std::array<size_t, static_cast<size_t>(Card::Suit::Count)> suitCount;
			suitCount.fill(0);
			for (const Card& card : cards)
				++suitCount[static_cast<size_t>(card.GetSuit())];

			std::sort(cards.begin(), cards.end(), [trumpSuit, &suitCount](const Card& a, const Card& b)
				{
					if (a.IsTrump(trumpSuit) != b.IsTrump(trumpSuit))
						return b.IsTrump(trumpSuit); // trump suit -> low priority

					if (a.GetRank() == b.GetRank())
						return suitCount[static_cast<size_t>(a.GetSuit())] > suitCount[static_cast<size_t>(b.GetSuit())]; // more common suits -> high priority

					return a.GetRank() < b.GetRank();
				});
		}

		static double getDiscardDeckRatio(const Deck& deck)
		{
			return static_cast<double>(deck.GetMaxCount() - deck.GetCount()) / deck.GetMaxCount();
		}

	private:
		static std::optional<Card> pickCard(const Context& context, double pickTrumpChance, std::vector<Card>&& filteredCards)
		{
			if (filteredCards.empty())
				return std::nullopt;

			sort(filteredCards, context.GetTrumpSuit());
			const Card& card = filteredCards.front();

			if (card.IsTrump(context.GetTrumpSuit()))
			{
				const int chance = static_cast<int>(pickTrumpChance * 100);
				if (Random::GetNumber(100, 1) > chance)
					return std::nullopt;
			}
			return card;
		}
	};

	class HardBehavior : public MediumBehavior
	{
	public:
		using MediumBehavior::MediumBehavior;

	protected:
		std::optional<Card> pickAttackCard(const Context& context, const Player& defender, std::vector<Card>&& filteredCards) const override
		{
			if (const auto* defenderCards = _memory.GetPlayerCards(defender.GetId()))
			{
				// TODO
			}
			return MediumBehavior::pickAttackCard(context, defender, std::move(filteredCards));
		}

		std::optional<Card> pickDefendCard(const Context& context, const Player& attacker, std::vector<Card>&& filteredCards) const override
		{
			if (const auto* attackerCards = _memory.GetPlayerCards(attacker.GetId()))
			{
				// TODO
			}
			return MediumBehavior::pickDefendCard(context, attacker, std::move(filteredCards));
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
		return _behavior->PickAttackCard(context, defender, filter);
	return std::nullopt;
}

std::optional<Card> Bot::pickDefendCard(const Context& context, const Player& attacker, const CardFilter& filter) const
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
	if (_behavior)
		return _behavior->PickDefendCard(context, attacker, filter);
	return std::nullopt;
}