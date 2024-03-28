#include "UI.h"
#include <array>
#include <SFML/Graphics/RenderWindow.hpp>
#include "Drawing.h"
#include "Color.h"
#include "Hand.h"
#include "Mutex.h"
#include "Context.h"
#include "Round.h"

namespace
{
	constexpr float eps = 1.e-5f;

	inline sf::Vector2f getCardSize()
	{
		return Screen::Card{}.getSize();
	}

	inline size_t findMinDenominator(size_t n, size_t start = 2)
	{
		for (size_t i = start; i < n; ++i)
		{
			if (n % i == 0)
				return i;
		}
		return n;
	}

	class ViewRestorer
	{
	public:
		ViewRestorer(sf::RenderTarget& target)
			: _target(target)
		{}

		~ViewRestorer()
		{
			_target.setView(_target.getDefaultView());
		}

	private:
		sf::RenderTarget& _target;
	};

	struct State
	{
		sf::Vector2f position;
		float angleDegree = 0.f;

		bool operator==(const State& other) const
		{
			return std::abs(position.x - other.position.x) < eps
				&& std::abs(position.y - other.position.y) < eps
				&& std::abs(angleDegree - other.angleDegree) < eps;
		}
	};

	struct Animation
	{
		State finalState;
		State speedMs;
	};

	class VisibleCard final
	{
	public:
		inline VisibleCard(const ::Card& card, const State& state)
			: _cardInfo(card)
			, _state(state)
		{}
		
		void SetOpen(bool open)
		{
			_open = open;
		}

		bool Draw(sf::Int32 msDelta, sf::RenderTarget& target)
		{
			std::unique_ptr<Screen::Card> screenCard;
			if (_open)
				screenCard = std::make_unique<Screen::OpenCard>(_cardInfo);
			else
				screenCard = std::make_unique<Screen::CloseCard>();

			if (_animation)
			{
				if (_animation->finalState == _state)
				{
					_animation.reset();
				}
				else
				{
					sf::Vector2f dir = _animation->finalState.position - _state.position;
					dir /= std::hypot(dir.x, dir.y);
					dir.x *= static_cast<float>(msDelta) * _animation->speedMs.position.x;
					dir.y *= static_cast<float>(msDelta) * _animation->speedMs.position.y;

					_state.position += dir;
					_state.angleDegree += static_cast<float>(msDelta) * _animation->speedMs.angleDegree;
				}
			}

			screenCard->setOrigin(_state.position);
			screenCard->setRotation(_state.angleDegree);
			target.draw(*screenCard);

			return !_animation;
		}

		void StartAnimation(const Animation& animation)
		{
			_animation = animation;
		}

		const State& GetState() const
		{
			return _state;
		}

	private:
		::Card _cardInfo;
		State _state;
		std::optional<Animation> _animation;
		bool _open = false;
	};

	class VisibleCards final
	{
	public:
		void StartAnimation(const ::Card& card, const Animation& animation)
		{
			auto iter = _map.find(card);
			if (iter != _map.end())
				iter->second.StartAnimation(animation);
		}

		bool Draw(sf::Int32 msDelta, sf::RenderTarget& target)
		{
			bool res = true;
			for (auto& [cardInfo, visibleCard] : _map)
				res = visibleCard.Draw(msDelta, target) && res;
			return res;
		}

		size_t GetCount() const
		{
			return _map.size();
		}

		State GetState(const ::Card& card) const
		{
			auto iter = _map.find(card);
			if (iter != _map.end())
				return iter->second.GetState();
			return {};
		}

	private:
		struct CardHash
		{
			size_t operator()(const Card& card) const
			{
				size_t result = static_cast<size_t>(card.GetSuit()) << 8;
				result += static_cast<size_t>(card.GetRank());
				return result;
			}
		};

		struct CardsEqual
		{
			bool operator()(const Card& a, const Card& b) const
			{
				return a == b;
			}
		};
		using Map = std::unordered_map<::Card, VisibleCard, CardHash, CardsEqual>;

		Map _map;
	};
}

struct UI::Data
{
	struct Flag
	{
		enum Value : int
		{
			Default			= 0,
			PickingCard		= 1 << 0,
			NeedRedraw		= 1 << 1,
		};
	};

	std::map<Player::Id, VisibleCards> playerCards;
	VisibleCards roundCards;
	std::underlying_type_t<Flag::Value> flags = Flag::Default;
	sf::Vector2f cursorPosition;
};

UI::UI(const std::string& title, unsigned int width, unsigned int height)
	: _window(sf::VideoMode{ width, height }, sf::String(title), sf::Style::Close)
	, _data(std::make_unique<Data>())
{
}

UI::~UI()
{
}

sf::RenderWindow& UI::GetWindow()
{
	return const_cast<sf::RenderWindow&>(const_cast<const UI*>(this)->GetWindow());
}

const sf::RenderWindow& UI::GetWindow() const
{
	return _window;
}

bool UI::NeedsToUpdate() const
{
	return _data && _data->flags & Data::Flag::NeedRedraw;
}

void UI::Update(const Context& context, sf::Int32 msDelta)
{
	if (!NeedsToUpdate())
		return;

	Mutex::Guard guard(Mutex::Get());
	const auto size = _window.getView().getSize();

	{
		Screen::Table table;
		_window.draw(table);
	}

	{
		Screen::Deck deck(context.GetDeck());
		deck.setOrigin(0.9f * size.x, 0.5f * size.y);
		_window.draw(deck);
	}

	if (_data->flags & Data::Flag::PickingCard)
	{
		Screen::SkipButton skipButton;
		skipButton.setOrigin(0.5f * size.x, size.y - Screen::Card{}.getSize().y);
		_window.draw(skipButton);
	}

	bool finished = true;
	for (auto& [playerId, playerCards] : _data->playerCards)
		finished = playerCards.Draw(msDelta, _window) && finished;

	finished = _data->roundCards.Draw(msDelta, _window) && finished;

	if (finished)
		_data->flags &= ~Data::Flag::NeedRedraw;
}

bool UI::HandleEvent(const sf::Event& event)
{
	Mutex::Guard guard(Mutex::Get());
	switch (event.type)
	{
	case sf::Event::EventType::MouseButtonPressed:
		if (_data->flags & Data::Flag::PickingCard)
		{
			

			_data->flags |= Data::Flag::NeedRedraw;
		}
		break;

	case sf::Event::EventType::MouseButtonReleased:
		_data->flags |= Data::Flag::NeedRedraw;
		//if (_data->interactZone.intersects()
		break;

	case sf::Event::EventType::MouseMoved:
		_data->cursorPosition = toModel({ event.mouseMove.x, event.mouseMove.y });
		_data->flags |= Data::Flag::NeedRedraw;
		break;
	}

	return false;
}

bool UI::IsLocked() const
{
	return _data && (_data->flags & Data::Flag::NeedRedraw);
}

void UI::OnRoundStart(const Round& round)
{
	_data->flags |= Data::Flag::NeedRedraw;
	_data->playerCards.clear();
}

void UI::OnPlayerAttack(const Player& attacker, const Card& attackCard)
{
	onPlayerPlaceCard(attacker, attackCard);
}

void UI::OnPlayerDefend(const Player& defender, const Card& attackCard, const Card& defendCard)
{
	onPlayerPlaceCard(defender, attackCard, defendCard);
}

void UI::OnRoundEnd(const Round& round)
{
	_data->flags |= Data::Flag::NeedRedraw;
}

void UI::OnPlayerDrawCards(const Player& player, const Deck& deck)
{
	_data->flags |= Data::Flag::NeedRedraw;
}

void UI::OnPlayerDrawCards(const Player& player, const Round& round)
{
	_data->flags |= Data::Flag::NeedRedraw;
}

void UI::OnStartGame(const Player& first, const Context& context)
{
	_data->flags |= Data::Flag::NeedRedraw;
}

void UI::OnUserWin(const Player& user, const Context& context)
{
	_data->flags |= Data::Flag::NeedRedraw;
}

void UI::OnUserLose(const Player& opponent, const Context& context)
{
	_data->flags |= Data::Flag::NeedRedraw;
}

std::optional<Card> UI::UserPickCard(const User& user)
{
	std::optional<Card> card;
	bool skip = false;

	_data->flags |= Data::Flag::PickingCard;
	while (true)
	{
		// ...

		if (card || skip)
			break;
	}
	_data->flags &= ~Data::Flag::PickingCard;
	return card;
}

sf::Vector2f UI::toModel(const sf::Vector2i& screen) const
{
	return _window.mapPixelToCoords(screen);
}

sf::Vector2i UI::toScreen(const sf::Vector2f& model) const
{
	return _window.mapCoordsToPixel(model);
}

sf::Vector2f UI::findRoundCardPlace(const std::optional<Card>& attackCard) const
{
	if (!_data)
		return {};

	const size_t roundCardCount = _data->roundCards.GetCount();
	if (roundCardCount >= Round::MaxAttacksCount * 2)
		return {};

	const size_t rows = findMinDenominator(Round::MaxAttacksCount, 2);
	const size_t columns = Round::MaxAttacksCount / rows;

	const size_t index = roundCardCount;
	const bool isOverlayed = index % 2 == 0;
	const size_t pair = index / 2;

	const size_t column = pair % rows;
	const size_t row = pair / rows;

	const sf::Vector2f cardSize = getCardSize();
	const sf::Vector2f overlapOffset = { cardSize.x * 0.2f, 0.f };
	const sf::Vector2f cardPairSize = cardSize + overlapOffset;
	const sf::Vector2f roundArea = 0.8f * _window.getView().getSize();
	const sf::Vector2f gap = { (roundArea.x - columns * cardPairSize.x) / (columns - 1), (roundArea.y - rows * cardPairSize.y) / (rows - 1) };

	sf::Vector2f position;
	position.x = column * cardPairSize.x + std::max(column - 1, (size_t)0) * gap.x;
	position.y = row * cardPairSize.y + std::max(row - 1, (size_t)0) * gap.y;
	position += 0.f * cardSize;
	if (isOverlayed)
		position += overlapOffset;
	return position;
}

void UI::onPlayerPlaceCard(const Player& player, const Card& attackCard, const std::optional<Card>& defendCard)
{
	auto iter = _data->playerCards.find(player.GetId());
	if (iter != _data->playerCards.end())
	{
		_data->flags |= Data::Flag::NeedRedraw;

		auto& cards = iter->second;
		const bool isDefending = defendCard.has_value();
		const auto& playerCard = defendCard ? defendCard.value() : attackCard;
		sf::Vector2f finalPosition;
		if (isDefending)
			finalPosition = findRoundCardPlace(attackCard);
		else
			finalPosition = findRoundCardPlace();

		Animation animation;
		animation.finalState.position = finalPosition;
		animation.finalState.angleDegree = 0.f;
		animation.speedMs.angleDegree = 0.006f;
		animation.speedMs.position = { 0.003f, 0.003f };
		cards.StartAnimation(playerCard, animation);

		// TODO: move other cards closer to each other
	}
}