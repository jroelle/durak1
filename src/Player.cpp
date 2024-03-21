#include "Player.h"
#include "Deck.h"

std::optional<Card> Player::Attack(const Player& opponent)
{
	const auto attackCard = pickAttackCard(opponent);
	removeCard(attackCard);
	return attackCard;
}

std::optional<Card> Player::Defend(const Player& opponent, const Card& attackCard)
{
	const auto defendCard = pickDefendCard(opponent, attackCard);
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
			if (card.IsTrump(trumpSuit) && (!lowest || *lowest > trumpSuit))
				lowest = card;

			return lowest && lowest->GetRank() == Card::Rank::Min;
		});
	return lowest;
}

void Player::removeCard(const std::optional<Card>& card)
{
	if (card)
		return _hand.RemoveCard(*card);
}