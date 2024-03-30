#include "Player.h"
#include "Deck.h"
#include "Context.h"
#include "Event.hpp"

Player::Player(Id id)
	: _id(id)
{}

std::optional<Card> Player::Attack(const Context& context)
{
	const auto attackCard = pickAttackCard(context);
	if (attackCard)
	{
		removeCard(attackCard);
		EventHandlers::Get().OnPlayerAttack(*this, *attackCard);
	}
	return attackCard;
}

std::optional<Card> Player::Defend(const Context& context, const Card& attackCard)
{
	const auto defendCard = pickDefendCard(context, attackCard);
	if (defendCard)
	{
		removeCard(defendCard);
		EventHandlers::Get().OnPlayerDefend(*this, *defendCard);
	}
	return defendCard;
}

Player& Player::DrawCards(Deck& deck)
{
	std::vector<Card> eventCards;
	eventCards.reserve(Hand::MinCount - _hand.GetCardCount());

	for (size_t i = _hand.GetCardCount(); i < Hand::MinCount && !deck.IsEmpty(); ++i)
	{
		const auto card = deck.PopFirst();
		_hand.AddCard(*card);
		eventCards.push_back(*card);
	}

	EventHandlers::Get().OnPlayerDrawDeckCards(*this, eventCards);
	return *this;
}

Player& Player::DrawCards(std::vector<Card>&& cards)
{
	EventHandlers::Get().OnPlayerDrawRoundCards(*this, cards);
	_hand.AddCards(std::make_move_iterator(cards.begin()), std::make_move_iterator(cards.end()));
	return *this;
}

std::optional<Card> Player::FindLowestTrumpCard(Card::Suit trumpSuit) const
{
	std::optional<Card> lowest;
	_hand.ForEachCard([&lowest, trumpSuit](const Card& card)
		{
			if (card.IsTrump(trumpSuit) && (!lowest || card.GetRank() < lowest->GetRank()))
				lowest.emplace(card);

			return lowest && lowest->GetRank() == Card::Rank::Min;
		});
	return lowest;
}

Player::Id Player::GetId() const
{
	return _id;
}

void Player::removeCard(const std::optional<Card>& card)
{
	if (card)
		_hand.RemoveCard(*card);
}