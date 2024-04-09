#include "UI.h"
#include <queue>
#include <set>
#include <numbers>
#include <thread>
#include <SFML/Graphics/RenderWindow.hpp>
#include "Utility.hpp"
#include "Drawing.h"
#include "Color.h"
#include "Hand.h"
#include "Context.h"
#include "Round.h"
#include "Player.h"
#include "PlayersGroup.h"

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
		static constexpr float tolerance = 1.e-2f;

		sf::Vector2f position;
		float angleDegree = 0.f;

		bool operator==(const State& other) const
		{
			return std::abs(position.x - other.position.x) < tolerance
				&& std::abs(position.y - other.position.y) < tolerance
				&& std::abs(angleDegree - other.angleDegree) < tolerance;
		}
	};

	struct Animation
	{
		using OnStart = std::function<void()>;
		using OnFinish = std::function<void()>;

		State finalState;
		float deltaPerFrame = 25.f;
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

				if (_state == animation.finalState)
				{
					_state = animation.finalState;
					if (animation.onFinish)
						animation.onFinish();
					_animations.pop();
				}
				else
				{
					sf::Vector2f dir = animation.finalState.position - _state.position;
					dir /= animation.deltaPerFrame;

					float angleDelta = animation.finalState.angleDegree - _state.angleDegree;
					angleDelta /= animation.deltaPerFrame;

					_state.position += dir;
					_state.angleDegree += angleDelta;
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

		bool IsPointInside(const sf::Vector2f& point, float interactOffset = 0.f) const
		{
			const auto cardSize = ::getCardSize();
			return ::isPointInRectange(_state.position, cardSize.x, cardSize.y, point, interactOffset);
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

		virtual std::optional<Card> Pick(const sf::Vector2f& cursor, float interactOffset = 0.f) const
		{
			return std::nullopt;
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

		const sf::Vector2f& GetPosition() const
		{
			return _position;
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

		std::optional<Card> Pick(const sf::Vector2f& cursor, float interactOffset = 0.f) const override
		{
			std::optional<Card> pick;
			_cards.for_each([&pick, &cursor, interactOffset](const VisibleCard& visibleCard)
				{
					if (visibleCard.IsPointInside(cursor, interactOffset))
						pick.emplace(visibleCard.GetCardInfo());
					return pick.has_value();
				});
			return pick;
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

		std::optional<Card> Pick(const sf::Vector2f& cursor, float interactOffset = 0.f) const
		{
			for (Player::Id id = 0; id < static_cast<Player::Id>(_players.size()); ++id)
			{
				if (auto pick = _players[id]->Pick(cursor, interactOffset))
					return pick;
			}
			return std::nullopt;
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
			UserPickingCard	= 1 << 0,
			UserCanSkip		= 1 << 1,
			NeedRedraw		= 1 << 2,
		};
	};

	struct UserPick
	{
		std::optional<Card> card;
	};

	Players playerCards;
	RoundCards roundCards;
	std::underlying_type_t<Flag::Value> flags = Flag::Default;
	sf::Vector2f cursorPosition;
	std::optional<UserPick> userPick;

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

bool UI::HandleEvent(const sf::Event& event)
{
	if (!_data)
		return false;

	switch (event.type)
	{
	case sf::Event::EventType::MouseButtonPressed:
		if ((_data->flags & Data::Flag::UserPickingCard)
			&& _data->userPick.has_value())
		{
			_data->flags |= Data::Flag::NeedRedraw;
			_data->flags &= ~Data::Flag::UserPickingCard;
			_data->flags &= ~Data::Flag::UserCanSkip;
		}
		break;

	//case sf::Event::EventType::MouseButtonReleased:
	//	_data->flags |= Data::Flag::NeedRedraw;
	//	break;

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

std::optional<Card> UI::UserPickCard(const Context& context, const User& user, bool canSkip)
{
	_data->flags |= Data::Flag::UserPickingCard | Data::Flag::UserCanSkip;

	while (_data->flags & Data::Flag::UserPickingCard)
		animate(context);

	_data->flags &= ~Data::Flag::UserCanSkip;
	if (_data->userPick && _data->userPick->card)
		return _data->userPick->card;

	return std::nullopt;
}

void UI::OnPlayerAttack(const Context& context, const Player& attacker, const Card& attackCard)
{
	onPlayerPlaceCard(context, attacker, attackCard);
}

void UI::OnPlayerDefend(const Context& context, const Player& defender, const Card& defendCard)
{
	onPlayerPlaceCard(context, defender, defendCard);
}

void UI::OnPlayerDrawDeckCards(const Context& context, const Player& player, const std::vector<Card>& cards)
{
	PlayerCards& playerCards = _data->playerCards.GetCards(player.GetId());
	const sf::Vector2f startPosition = getDeckPosition();
	for (const Card& cardInfo : cards)
	{
		playerCards.Add(VisibleCard(cardInfo, State{ startPosition, 0.f }));
	}
	animate(context);
}

void UI::OnPlayerDrawRoundCards(const Context& context, const Player& player, const std::vector<Card>& cards)
{
	PlayerCards& playerCards = _data->playerCards.GetCards(player.GetId());
	for (const auto& cardInfo : cards)
	{
		playerCards.MoveFrom(cardInfo, _data->roundCards);
	}
	animate(context);
}

void UI::OnRoundStart(const Context& context, const Round& round)
{
	const auto& attackerCards = _data->playerCards.GetCards(round.GetAttacker().GetId());
	const auto& defenderCards = _data->playerCards.GetCards(round.GetDefender().GetId());
	const sf::Vector2f& arrowStart = attackerCards.GetPosition();
	const sf::Vector2f& arrowEnd = defenderCards.GetPosition();
	// TODO: draw attacker to defender arrow

	animate(context);
}

void UI::OnRoundEnd(const Context& context, const Round& round)
{
	_data->roundCards.RemoveAll();
	animate(context);
}

void UI::OnPlayersCreated(const Context& context, const PlayersGroup& players)
{
	_data = std::make_unique<Data>(_window.getView(), players.GetCount() - 1);
	animate(context);
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
}

void UI::OnStartGame(const Context& context, const Player& first)
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
	animate(context);
}

void UI::OnUserWin(const Context& context, const Player& user)
{
	animate(context);
}

void UI::OnUserLose(const Context& context, const Player& opponent)
{
	animate(context);
}

sf::Vector2f UI::toModel(const sf::Vector2i& screen) const
{
	return _window.mapPixelToCoords(screen);
}

sf::Vector2i UI::toScreen(const sf::Vector2f& model) const
{
	return _window.mapCoordsToPixel(model);
}

void UI::onPlayerPlaceCard(const Context& context, const Player& player, const Card& card)
{
	_data->roundCards.MoveFrom(card, _data->playerCards.GetCards(player.GetId()));
	animate(context);
}

sf::Vector2f UI::getDeckPosition() const
{
	const auto size = _window.getView().getSize();
	return { 0.9f * size.x, 0.5f * size.y };
}

void UI::animate(const Context& context)
{
	sf::Clock clock;
	_data->flags |= Data::Flag::NeedRedraw;
	while (NeedsToUpdate())
	{
		_window.clear();
		update(context, clock.restart());
		_window.display();
	}
}

void UI::update(const Context& context, sf::Time delta)
{
	if (!NeedsToUpdate())
		return;

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

	_data->userPick = {};
	if (_data->flags & Data::Flag::UserPickingCard)
	{
		Screen::SkipButton skipButton;
		const sf::Vector2f center(0.5f * size.x, size.y - Screen::Card{}.getSize().y);
		skipButton.setOrigin(center);
		_window.draw(skipButton);
		if ((_data->flags & Data::Flag::UserCanSkip) &&
			::isPointInRectange(center, Screen::SkipButton::Size, Screen::SkipButton::Size, _data->cursorPosition, interactOffset))
		{
			_data->userPick.emplace();
			cursorType = sf::Cursor::Hand;
		}
		else if (auto pick = _data->playerCards.Pick(_data->cursorPosition, interactOffset))
		{
			_data->userPick.emplace(std::move(pick));
			cursorType = sf::Cursor::Hand;
		}
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