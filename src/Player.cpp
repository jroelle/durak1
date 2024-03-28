#include "Player.h"
#include "Deck.h"
#include "Context.h"

Player::Player(Id id)
	: _id(id)
{}

std::optional<Card> Player::Attack(const Context& context)
{
	const auto attackCard = pickAttackCard(context);
	removeCard(attackCard);
	return attackCard;
}

std::optional<Card> Player::Defend(const Context& context, const Card& attackCard)
{
	const auto defendCard = pickDefendCard(context, attackCard);
	removeCard(defendCard);
	return defendCard;
}

Player& Player::DrawCards(Deck& deck)
{
	for (size_t i = _hand.GetCardCount(); i < Hand::MinCount && !deck.IsEmpty(); ++i)
		_hand.AddCard(*deck.PopFirst());
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

size_t Player::GetCardCount() const
{
	return _hand.GetCardCount();
}

bool Player::HasAnyCards() const
{
	return !_hand.IsEmpty();
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