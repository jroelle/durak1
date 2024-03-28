#include "UI.h"
#include <queue>
#include <SFML/Graphics/RenderWindow.hpp>
#include "Utility.hpp"
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

	inline bool isPointInRectange(const sf::Vector2f& origin, float width, float height, const sf::Vector2f& point, float offset = 0.f)
	{
		return origin.x - 0.5f * width - offset < point.x && point.x < origin.x + 0.5f * width + offset
			&& origin.y - 0.5f * height - offset < point.y && point.y < origin.y + 0.5f * height + offset;
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
		using OnFinish = std::function<void()>;

		State finalState;
		State speedMs;
		OnFinish onFinish;
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

			if (!_animations.empty())
			{
				if (_animations.front().finalState == _state)
					_animations.pop();

				if (!_animations.empty())
				{
					const auto& animation = _animations.front();
					sf::Vector2f dir = animation.finalState.position - _state.position;
					dir /= std::hypot(dir.x, dir.y);
					dir.x *= static_cast<float>(msDelta) * animation.speedMs.position.x;
					dir.y *= static_cast<float>(msDelta) * animation.speedMs.position.y;

					_state.position += dir;
					_state.angleDegree += static_cast<float>(msDelta) * animation.speedMs.angleDegree;
				}
			}

			screenCard->setOrigin(_state.position);
			screenCard->setRotation(_state.angleDegree);
			target.draw(*screenCard);

			return _animations.empty();
		}

		void StartAnimation(const Animation& animation)
		{
			_animations.push(animation);
		}

		const State& GetState() const
		{
			return _state;
		}

		const ::Card& GetCardInfo() const
		{
			return _cardInfo;
		}

	private:
		::Card _cardInfo;
		State _state;
		std::queue<Animation> _animations;
		bool _open = false;
	};

	class VisibleCards
	{
	public:
		virtual ~VisibleCards() = default;

		void StartAnimation(const ::Card& cardInfo, const Animation& animation)
		{
			if (auto* visibleCard = _list.find(cardInfo))
				visibleCard->StartAnimation(animation);
		}

		bool Draw(sf::Int32 msDelta, sf::RenderTarget& target)
		{
			bool res = true;
			_list.for_each([&res, msDelta, &target](VisibleCard* visibleCard)
				{
					res = visibleCard->Draw(msDelta, target) && res;
					return false;
				});
			return res;
		}

		size_t GetCount() const
		{
			return _list.size();
		}

		State GetState(const ::Card& cardInfo) const
		{
			if (const auto* visibleCard = _list.find(cardInfo))
				return visibleCard->GetState();
			return {};
		}

	private:
		struct Hash
		{
			size_t operator()(const ::Card& card) const
			{
				size_t result = static_cast<size_t>(card.GetSuit()) << 8;
				result += static_cast<size_t>(card.GetRank());
				return result;
			}

			size_t operator()(const VisibleCard& visibleCard) const
			{
				return operator()(visibleCard.GetCardInfo());
			}
		};

		struct Equal
		{
			bool operator()(const ::Card& a, const ::Card& b) const
			{
				return a == b;
			}
			bool operator()(const VisibleCard& a, const VisibleCard& b) const
			{
				return operator()(a.GetCardInfo(), b.GetCardInfo());
			}
		};

		using List = utility::loop_list<VisibleCard, Hash, Equal>;
		List _list;
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

	constexpr float interactOffset = 2.5f;
	const auto size = _window.getView().getSize();
	sf::Cursor::Type cursorType = sf::Cursor::Arrow;

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
		const sf::Vector2f center(0.5f * size.x, size.y - Screen::Card{}.getSize().y);
		skipButton.setOrigin(center);
		_window.draw(skipButton);
		if (isPointInRectange(center, Screen::SkipButton::Size, Screen::SkipButton::Size, _data->cursorPosition, interactOffset))
			cursorType = sf::Cursor::Hand;
	}

	bool finished = true;
	for (auto& [playerId, playerCards] : _data->playerCards)
		finished = playerCards.Draw(msDelta, _window) && finished;

	finished = _data->roundCards.Draw(msDelta, _window) && finished;

	if (finished)
		_data->flags &= ~Data::Flag::NeedRedraw;

	if (_cursorType != cursorType)
	{
		_cursorType = cursorType;
		sf::Cursor cursor;
		cursor.loadFromSystem(cursorType);
		_window.setMouseCursor(cursor);
	}
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

	// remove cards from the table
}

void UI::OnPlayerDrawCards(const Player& player, const Deck& deck)
{
	_data->flags |= Data::Flag::NeedRedraw;

	auto iter = _data->playerCards.find(player.GetId());
	if (iter != _data->playerCards.end())
	{
		auto& cards = iter->second;
		cards.StartAnimation();
	}
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