#include "UI.h"
#include <queue>
#include <set>
#include <numbers>
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

	inline float length(const sf::Vector2f& v)
	{
		return std::hypot(v.x, v.y);
	}

	inline float dotProduct(const sf::Vector2f& a, const sf::Vector2f& b)
	{
		return a.x * b.x + a.y * b.y;
	}

	inline float angleDegree(const sf::Vector2f& a, const sf::Vector2f& b)
	{
		const float cos = dotProduct(a, b) / (length(a) * length(b));
		const float angle = std::acos(cos) * 180.f / std::numbers::pi_v<float>;
		const float remains = angle / 180.f - std::trunc(angle / 180.f);
		return remains * 180.f;
	}

	inline sf::Vector2f rotate(const sf::Vector2f& v, float angleDegree)
	{
		const float cos = std::cos(angleDegree * std::numbers::pi_v<float> / 180.f);
		const float sin = std::sin(angleDegree * std::numbers::pi_v<float> / 180.f);

		return sf::Vector2f{ v.x * cos - v.y * sin, v.x * sin + v.y * cos };
	}

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
		using OnStart = std::function<void()>;
		using OnFinish = std::function<void()>;

		State finalState;
		sf::Time time = sf::milliseconds(2000);
		OnStart onStart;
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

		bool Draw(sf::Time delta, sf::RenderTarget& target)
		{
			std::unique_ptr<Screen::Card> screenCard;
			if (_open)
				screenCard = std::make_unique<Screen::OpenCard>(_cardInfo);
			else
				screenCard = std::make_unique<Screen::CloseCard>();

			bool hasAnimations = !_animations.empty();
			if (!_animations.empty())
			{
				auto& animation = _animations.front();
				if (animation.onStart)
				{
					animation.onStart();
					animation.onStart = {};
				}

				sf::Vector2f dir = animation.finalState.position - _state.position;
				dir /= static_cast<float>(delta.asMilliseconds());

				float angleDelta = animation.finalState.angleDegree - _state.angleDegree;
				angleDelta /= static_cast<float>(delta.asMilliseconds());

				_state.position += dir;
				_state.angleDegree += angleDelta;

				if (animation.time > delta)
				{
					animation.time -= delta;
				}
				else
				{
					if (animation.onFinish)
						animation.onFinish();
					_animations.pop();
				}
			}

			screenCard->setOrigin(_state.position);
			screenCard->setRotation(_state.angleDegree);
			target.draw(*screenCard);

			return !hasAnimations;
		}

		void StartAnimation(const Animation& animation)
		{
			_animations.push(animation);
		}

		void ResetAnimation()
		{
			_animations = {};
		}

		const State& GetState() const
		{
			return _state;
		}

		const State& GetFinalState() const
		{
			return _animations.empty() ? _state : _animations.back().finalState;
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
		using ForEachCard = std::function<void(const ::Card&)>;

		VisibleCards(const sf::View& view)
			: _view(view)
		{}

		virtual ~VisibleCards() = default;

		void MoveFrom(const ::Card& cardInfo, VisibleCards& other)
		{
			Add(other.Remove(cardInfo));
		}

		void Add(VisibleCard&& visibleCard)
		{
			const Card key = visibleCard.GetCardInfo();
			_cards.push_back(key, std::move(visibleCard));
			onCardAdded(_cards.at(key));
		}

		VisibleCard Remove(const ::Card& cardInfo)
		{
			VisibleCard visibleCard = _cards.remove(cardInfo);
			onCardRemoved(visibleCard);
			return visibleCard;
		}

		void StartAnimation(const Animation& animation)
		{
			_cards.for_each([&animation](VisibleCard& visibleCard)
				{
					visibleCard.StartAnimation(animation);
					return false;
				});
		}

		void StartAnimation(const ::Card& cardInfo, const Animation& animation)
		{
			_cards.at(cardInfo).StartAnimation(animation);
		}

		bool Draw(sf::Time delta, sf::RenderTarget& target)
		{
			bool res = true;
			_cards.for_each([&res, delta, &target](VisibleCard& visibleCard)
				{
					res = visibleCard.Draw(delta, target) && res;
					return false;
				});
			return res;
		}

		size_t GetCount() const
		{
			return _cards.size();
		}

		std::optional<State> GetState(const ::Card& cardInfo) const
		{
			return _cards.at(cardInfo).GetState();
		}

	private:
		virtual void onCardAdded(VisibleCard&) {}
		virtual void onCardRemoved(VisibleCard&) {}

	protected:
		struct Hash
		{
			size_t operator()(const Card& card) const
			{
				size_t hash = static_cast<size_t>(card.GetSuit()) << 8;
				hash += static_cast<size_t>(card.GetRank());
				return hash;
			}
		};

		using Storage = utility::mapped_list<Card, VisibleCard, Hash>;
		Storage _cards;
		const sf::View _view;
	};

	class RoundCards final : public VisibleCards
	{
	public:
		using VisibleCards::VisibleCards;

		void RemoveAll()
		{
			Animation animation;
			animation.finalState.position = { -200.f, -200.f };
			StartAnimation(animation);
		}

	private:
		void onCardAdded(VisibleCard& cardAdded)
		{
			cardAdded.SetOpen(true);

			Animation animation;
			animation.finalState = getNewCardState();
			cardAdded.StartAnimation(animation);
		}

		State getNewCardState() const
		{
			const size_t roundCardCount = _cards.size();
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
			const sf::Vector2f roundArea = 0.6f * _view.getSize();
			const sf::Vector2f gap = { (roundArea.x - columns * cardPairSize.x) / (columns - 1), (roundArea.y - rows * cardPairSize.y) / (rows - 1) };

			const sf::Vector2f start = 0.5f * (_view.getSize() - roundArea);

			sf::Vector2f position = start;
			position.x += column * cardPairSize.x + column * gap.x;
			position.y += row * cardPairSize.y + row * gap.y;
			if (isOverlayed)
				position += overlapOffset;

			return { position, 0.f };
		}
	};

	class PlayerCards : public VisibleCards
	{
	public:
		PlayerCards(const sf::View& view, const sf::Vector2f& position, const sf::Vector2f& faceDirection)
			: VisibleCards(view)
			, _position(position)
			, _faceDirection(faceDirection)
		{}

		virtual ~PlayerCards() = default;

		void ShowCard(const Card& cardInfo)
		{
			auto& visibleCard = _cards.at(cardInfo);
			const auto& state = visibleCard.GetFinalState();
			{
				Animation animation;
				animation.finalState.position = state.position + 50.f * _faceDirection;
				animation.finalState.angleDegree = state.angleDegree;
				animation.onStart = [&]()
					{
						visibleCard.SetOpen(true);
					};
				visibleCard.StartAnimation(animation);
			}

			{
				Animation animation;
				animation.finalState = state;
				animation.onFinish = [&]()
					{
						visibleCard.SetOpen(IsOpen());
					};
				visibleCard.StartAnimation(animation);
			}
		}

		std::optional<Card> FindCardUnderCursor(const sf::Vector2f& cursor) const
		{
			// TODO
			return std::nullopt;
		}

		virtual bool IsOpen() const
		{
			return false;
		}

	private:
		void onCardAdded(VisibleCard& cardAdded) override
		{
			cardAdded.SetOpen(IsOpen());
			moveCards();
		}

		void onCardRemoved(VisibleCard& cardRemoved) override
		{
			moveCards();
		}

	private:
		void moveCards()
		{
			if (_cards.empty())
				return;

			const float cardsCount = static_cast<float>(_cards.size());
			const sf::Vector2f cardSize = getCardSize();
			const sf::Vector2f dir = rotate(_faceDirection, 90.f);

			const float maxWidth = 0.6f * std::min(_view.getSize().x, _view.getSize().y);
			float gap = cardSize.x * 0.4f;
			const float cardsWidth = cardSize.x * cardsCount + (cardsCount - 1) * gap;
			const float offset = cardsWidth > maxWidth ? (maxWidth - cardSize.x) / (cardsCount - 1) : cardSize.x + gap;

			sf::Vector2f start = _position - dir * 0.5f * (std::min(maxWidth, cardsWidth) - cardSize.x);
			uint8_t i = 0;
			_cards.for_each([&](VisibleCard& visibleCard)
				{
					visibleCard.ResetAnimation();
					Animation animation;
					animation.finalState.position = start + dir * offset * static_cast<float>(i);
					animation.finalState.angleDegree = angleDegree({ 0.f, -1.f }, _faceDirection); // TODO: rotating around wrong center
					visibleCard.StartAnimation(animation);
					++i;
					return false;
				});
		}

	private:
		sf::Vector2f _position;
		sf::Vector2f _faceDirection;
	};

	class UserCards final : public PlayerCards
	{
	public:
		using PlayerCards::PlayerCards;

		bool IsOpen() const override
		{
			return true;
		}
	};

	class Players
	{
	public:
		Players(const sf::View& view, size_t botsNumber)
		{
			_players.reserve(botsNumber + 1);
			_players.push_back(std::make_unique<UserCards>(view, sf::Vector2f{ 0.5f * view.getSize().x, view.getSize().y }, sf::Vector2f{ 0.f, -1.f }));

			switch (botsNumber)
			{
			case 2:
				_players.push_back(std::make_unique<PlayerCards>(view, sf::Vector2f{ 0.f, 0.5f * view.getSize().y}, sf::Vector2f{ 1.f, 0.f }));
				[[fallthrough]];

			case 1:
				_players.push_back(std::make_unique<PlayerCards>(view, sf::Vector2f{ 0.5f * view.getSize().x, 0.f }, sf::Vector2f{ 0.f, 1.f }));
				break;
			// TODO
			}
		}

		size_t GetCount() const
		{
			return _players.size();
		}

		PlayerCards& GetCards(Player::Id id)
		{
			return *_players[id];
		}

		const PlayerCards& GetCards(Player::Id id) const
		{
			return *_players[id];
		}

		bool Draw(sf::Time delta, sf::RenderTarget& target)
		{
			bool res = true;
			for (auto& player : _players)
				res = player->Draw(delta, target) && res;
			return res;
		}

	private:
		using Index = Player::Id;
		std::vector<std::unique_ptr<PlayerCards>> _players;
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

	Players playerCards;
	RoundCards roundCards;
	std::underlying_type_t<Flag::Value> flags = Flag::Default;
	sf::Vector2f cursorPosition;

	Data(const sf::View& view, size_t botsNumber)
		: playerCards(view, botsNumber)
		, roundCards(view)
	{
	}
};

UI::UI(const std::string& title, unsigned int width, unsigned int height)
	: _window(sf::VideoMode{ width, height }, sf::String(title), sf::Style::Close)
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

void UI::Update(const Context& context, sf::Time delta)
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
		deck.setOrigin(getDeckPosition());
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
	finished = _data->playerCards.Draw(delta, _window) && finished;
	finished = _data->roundCards.Draw(delta, _window) && finished;

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

std::optional<Card> UI::UserPickCard(const User& user)
{
	std::optional<Card> card;
	bool skip = false;

	_data->flags |= Data::Flag::PickingCard;
	//while (_window.isOpen() && _data->flags & Data::Flag::PickingCard)
	//{
	//	// ...

	//	if (card || skip)
	//		break;
	//}
	_data->flags &= ~Data::Flag::PickingCard;
	return card;
}

void UI::OnPlayerAttack(const Player& attacker, const Card& attackCard)
{
	onPlayerPlaceCard(attacker, attackCard);
}

void UI::OnPlayerDefend(const Player& defender, const Card& defendCard)
{
	onPlayerPlaceCard(defender, defendCard);
}

void UI::OnPlayerDrawDeckCards(const Player& player, const std::vector<Card>& cards)
{
	PlayerCards& playerCards = _data->playerCards.GetCards(player.GetId());
	const sf::Vector2f startPosition = getDeckPosition();
	for (const Card& cardInfo : cards)
	{
		playerCards.Add(VisibleCard(cardInfo, State{ startPosition, 0.f }));
	}
	awaitUpdate();
}

void UI::OnPlayerDrawRoundCards(const Player& player, const std::vector<Card>& cards)
{
	PlayerCards& playerCards = _data->playerCards.GetCards(player.GetId());
	for (const auto& cardInfo : cards)
	{
		playerCards.MoveFrom(cardInfo, _data->roundCards);
	}
	awaitUpdate();
}

void UI::OnRoundStart(const Round& round)
{
	awaitUpdate();
}

void UI::OnRoundEnd(const Round& round)
{
	_data->roundCards.RemoveAll();
	awaitUpdate();
}

void UI::OnPlayersCreated(const PlayersGroup& players)
{
	_data = std::make_unique<Data>(_window.getView(), players.GetCount() - 1);
	awaitUpdate();
}

void UI::OnStartGame(const Player& first, const Context& context)
{
	const PlayersGroup& players = context.GetPlayers();
	const Card::Suit trumpSuit = context.GetTrumpSuit();
	players.ForEach([&](const Player* player)
		{
			if (const auto trumpCard = player->FindLowestTrumpCard(trumpSuit))
			{
				auto& playerCards = _data->playerCards.GetCards(player->GetId());
				playerCards.ShowCard(*trumpCard);
			}
			return false;

		}, players.GetUser());
	awaitUpdate();
}

void UI::OnUserWin(const Player& user, const Context& context)
{
	awaitUpdate();
}

void UI::OnUserLose(const Player& opponent, const Context& context)
{
	awaitUpdate();
}

sf::Vector2f UI::toModel(const sf::Vector2i& screen) const
{
	return _window.mapPixelToCoords(screen);
}

sf::Vector2i UI::toScreen(const sf::Vector2f& model) const
{
	return _window.mapCoordsToPixel(model);
}

void UI::onPlayerPlaceCard(const Player& player, const Card& card)
{
	_data->roundCards.MoveFrom(card, _data->playerCards.GetCards(player.GetId()));
	awaitUpdate();
}

sf::Vector2f UI::getDeckPosition() const
{
	const auto size = _window.getView().getSize();
	return { 0.9f * size.x, 0.5f * size.y };
}

void UI::awaitUpdate()
{
	_data->flags |= Data::Flag::NeedRedraw;
	//while (_data->flags & Data::Flag::NeedRedraw)
	//{
	//	// updating in another thread
	//}
}