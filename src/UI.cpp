#include "UI.h"
#include <queue>
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
#include "Vector.h"

namespace
{
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
		float deltaPerFrame = 15.f;
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

			const bool hasAnimations = HasAnimations();
			if (HasAnimations())
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
			return ::isPointInRectange(GetFinalState().position, cardSize.x, cardSize.y, point, interactOffset);
		}

		bool HasAnimations() const
		{
			return !_animations.empty();
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

		virtual bool Draw(sf::Time delta, sf::RenderTarget& target)
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

		virtual std::optional<Card> Pick(const sf::Vector2f& cursor, float interactOffset = 0.f, const std::function<bool(const Card&)>& filter = {}) const
		{
			return std::nullopt;
		}

	protected:
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

		bool Draw(sf::Time delta, sf::RenderTarget& target) override
		{
			const bool res = VisibleCards::Draw(delta, target);
			if (_clear)
			{
				_cards.clear();
				_clear = false;
			}
			return res;
		}

		void RemoveAll()
		{
			_cards.for_each([this](VisibleCard& visibleCard)
				{
					Animation animation;
					animation.finalState.position = { -getCardSize().x, _view.getSize().y * 0.5f };
					animation.onFinish = [this]()
						{
							_clear = true;
						};
					visibleCard.StartAnimation(animation);
					return false;
				});
		}

		std::optional<Card> GetLast() const
		{
			if (_cards.empty())
				return std::nullopt;

			return _cards.back().GetCardInfo();
		}

		bool IsEmpty() const
		{
			return _cards.empty();
		}

	protected:
		void onCardAdded(VisibleCard& cardAdded) override
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

			const size_t index = roundCardCount - 1;
			const bool isOverlayed = index % 2 == 0;
			const size_t pair = index / 2;

			const size_t column = pair % columns;
			const size_t row = pair / columns;

			const sf::Vector2f cardSize = getCardSize();
			const sf::Vector2f overlapOffset = { -cardSize.x * 0.2f, 0.f };
			const sf::Vector2f cardPairSize = cardSize + overlapOffset;
			const sf::Vector2f roundArea = 0.5f * _view.getSize();
			const sf::Vector2f gap = { (roundArea.x - columns * cardPairSize.x) / (columns - 1), (roundArea.y - rows * cardPairSize.y) / (rows - 1) };

			const sf::Vector2f start = 0.5f * (_view.getSize() - roundArea);

			sf::Vector2f position = start;
			position.x += column * (cardPairSize.x + gap.x);
			position.y += row * (cardPairSize.y + gap.y);
			if (isOverlayed)
				position += overlapOffset;

			return { position, 0.f };
		}

	private:
		bool _clear = false;
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
			constexpr float deltaCoeff = 0.5f;

			auto& visibleCard = _cards.at(cardInfo);
			const auto& state = visibleCard.GetFinalState();
			{
				Animation animation;
				animation.finalState.position = state.position + 200.f * _faceDirection;
				animation.finalState.angleDegree = state.angleDegree;
				animation.deltaPerFrame *= deltaCoeff;
				animation.onStart = [&]()
					{
						visibleCard.SetOpen(true);
					};
				visibleCard.StartAnimation(animation);
			}

			{
				Animation animation;
				animation.finalState = state;
				animation.deltaPerFrame *= deltaCoeff;
				animation.onFinish = [&]()
					{
						visibleCard.SetOpen(IsOpen());
						std::this_thread::sleep_for(std::chrono::seconds(1));
					};
				visibleCard.StartAnimation(animation);
			}
		}

		const sf::Vector2f& GetPosition() const
		{
			return _position;
		}

		const sf::Vector2f& GetFaceDirection() const
		{
			return _faceDirection;
		}

		virtual bool IsOpen() const
		{
			return false;
		}

		virtual void Hover(const std::optional<Card>& cardInfo)
		{
			
		}

	protected:
		struct DefaultStateOptions
		{
			sf::Vector2f start;
			sf::Vector2f dir;
			float offset = 0.f;
		};

		std::optional<DefaultStateOptions> getDefaultStateOptions() const
		{
			if (_cards.empty())
				return {};

			const float cardsCount = static_cast<float>(_cards.size());
			const sf::Vector2f cardSize = getCardSize();
			const sf::Vector2f dir = rotate(_faceDirection, 90.f);

			const float maxWidth = 0.6f * std::min(_view.getSize().x, _view.getSize().y);
			float gap = cardSize.x * 0.4f;
			const float cardsWidth = cardSize.x * cardsCount + (cardsCount - 1) * gap;
			const float offset = cardsWidth > maxWidth ? (maxWidth - cardSize.x) / (cardsCount - 1) : cardSize.x + gap;

			sf::Vector2f start = _position - dir * 0.5f * (std::min(maxWidth, cardsWidth) - cardSize.x);

			DefaultStateOptions options;
			options.start = start;
			options.dir = dir;
			options.offset = offset;
			return options;
		}

		std::optional<State> getDefaultState(size_t i, const std::optional<DefaultStateOptions>& options) const
		{
			std::optional<DefaultStateOptions> actualOptions;
			if (options)
				actualOptions = options;
			else
				actualOptions = getDefaultStateOptions();

			if (!actualOptions)
				return {};

			State state;
			state.position = actualOptions->start + actualOptions->dir * actualOptions->offset * static_cast<float>(i);
			state.angleDegree = angleDegree({ 0.f, -1.f }, _faceDirection, 180.f); // TODO: rotating around wrong center
			return state;
		}

		void onCardAdded(VisibleCard& cardAdded) override
		{
			cardAdded.SetOpen(IsOpen());
			arrangeCards();
		}

		void onCardRemoved(VisibleCard& cardRemoved) override
		{
			arrangeCards();
		}

		void arrangeCards()
		{
			const auto options = getDefaultStateOptions();
			if (!options)
				return;

			size_t i = 0;
			_cards.for_each([this, &options, &i](VisibleCard& visibleCard)
				{
					visibleCard.ResetAnimation();
					Animation animation;
					animation.finalState = *getDefaultState(i, options);
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

		std::optional<Card> Pick(const sf::Vector2f& cursor, float interactOffset = 0.f, const std::function<bool(const Card&)>& filter = {}) const override
		{
			for (auto iter = _cards.rbegin(); iter != _cards.rend(); ++iter)
			{
				const VisibleCard& visibleCard = *iter;
				if ((!filter || filter(visibleCard.GetCardInfo())) && visibleCard.IsPointInside(cursor, interactOffset))
					return visibleCard.GetCardInfo();
			}
			return std::nullopt;
		}

		void Hover(const std::optional<Card>& cardInfo)
		{
			constexpr float offset = 25.f;
			constexpr float animationCoeff = 0.5;

			const auto options = getDefaultStateOptions();
			if (!options)
				return;

			if (cardInfo && (!_hoverCard || *_hoverCard != *cardInfo))
			{
				auto& visibleCard = _cards.at(*cardInfo);
				const auto defaultState = getDefaultState(_cards.index_of(*cardInfo), options);

				Animation animation;
				animation.finalState = *defaultState;
				animation.finalState.position += GetFaceDirection() * offset;
				animation.deltaPerFrame *= animationCoeff;

				visibleCard.ResetAnimation();
				visibleCard.StartAnimation(animation);
			}
			if (_hoverCard && (!cardInfo || *_hoverCard != *cardInfo))
			{
				Animation animation;
				animation.finalState = *getDefaultState(_cards.index_of(*_hoverCard), options);
				animation.deltaPerFrame *= animationCoeff;

				auto& visibleCard = _cards.at(*_hoverCard);
				visibleCard.ResetAnimation();
				visibleCard.StartAnimation(animation);
			}
			_hoverCard = cardInfo;
		}

	private:
		void onCardRemoved(VisibleCard& cardRemoved) override
		{
			if (_hoverCard && *_hoverCard == cardRemoved.GetCardInfo())
				_hoverCard.reset();
			PlayerCards::onCardRemoved(cardRemoved);
		}

	private:
		std::optional<Card> _hoverCard;
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
			NeedRedraw		= 1 << 0,
			UserPickingCard = 1 << 1,
			ClickToStart	= 1 << 2,
			UserVictory		= 1 << 3,
			UserDefeat		= 1 << 4,
		};
	};

	struct UserPick
	{
		struct Options
		{
			PickCardFilter filter;
			bool attacking = false;
		};

		struct Result
		{
			std::optional<Card> card;
		};

		Options options;
		std::optional<Result> result;
	};

	struct Arrow
	{
		sf::Vector2f direction;
	};

	struct Game
	{
		Players playerCards;
		RoundCards roundCards;

		Game(const sf::View& view, size_t botsNumber)
			: playerCards(view, botsNumber)
			, roundCards(view)
		{}
	};

	std::unique_ptr<Game> game;
	std::underlying_type_t<Flag::Value> flags = Flag::Default;
	sf::Vector2f cursorPosition;
	std::optional<UserPick> userPick;
	std::optional<Arrow> arrow;
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
		if (_data->userPick.has_value() && (_data->flags & Data::Flag::UserPickingCard))
		{
			_data->flags &= ~Data::Flag::NeedRedraw;
			_data->flags &= ~Data::Flag::UserPickingCard;
		}
		else if (_data->flags & Data::Flag::ClickToStart)
		{
			_data->flags &= ~Data::Flag::ClickToStart;
		}
		break;

	case sf::Event::EventType::MouseMoved:
		_data->cursorPosition = toModel({ event.mouseMove.x, event.mouseMove.y });
		if (_data->flags & Data::Flag::UserPickingCard)
			_data->flags |= Data::Flag::NeedRedraw;
		break;
	}

	return false;
}

bool UI::IsLocked() const
{
	return _data && (_data->flags & Data::Flag::NeedRedraw);
}

std::optional<Card> UI::UserPickCard(const Context& context, bool attacking, const PickCardFilter& filter)
{
	if (!_data)
		return std::nullopt;

	auto& userPick = _data->userPick.emplace();
	userPick.options.filter = filter;
	userPick.options.attacking = attacking;
	_data->flags |= Data::Flag::UserPickingCard;

	while (_data->flags & Data::Flag::UserPickingCard)
		animate(context);

	std::optional<Card> card;
	if (userPick.result && userPick.result->card)
		card = userPick.result->card;

	_data->userPick.reset();
	return card;
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
	if (!_data || !_data->game)
		return;

	PlayerCards& playerCards = _data->game->playerCards.GetCards(player.GetId());
	const sf::Vector2f startPosition = getDeckPosition();
	for (const Card& cardInfo : cards)
	{
		playerCards.Add(VisibleCard(cardInfo, State{ startPosition, 0.f }));
	}
	animate(context);
}

void UI::OnPlayerDrawRoundCards(const Context& context, const Player& player, const std::vector<Card>& cards)
{
	if (!_data || !_data->game)
		return;

	PlayerCards& playerCards = _data->game->playerCards.GetCards(player.GetId());
	for (const auto& cardInfo : cards)
	{
		playerCards.MoveFrom(cardInfo, _data->game->roundCards);
	}
	animate(context);
}

void UI::OnRoundStart(const Context& context, const Round& round)
{
	if (!_data || !_data->game)
		return;

	const auto& attackerCards = _data->game->playerCards.GetCards(round.GetAttacker().GetId());
	const auto& defenderCards = _data->game->playerCards.GetCards(round.GetDefender().GetId());
	sf::Vector2f dir = defenderCards.GetPosition() - attackerCards.GetPosition();
	dir /= length(dir);
	_data->arrow.emplace(dir);

	animate(context);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	_data->arrow.reset();
}

void UI::OnRoundEnd(const Context& context, const Round& round)
{
	if (!_data || !_data->game)
		return;

	_data->game->roundCards.RemoveAll();
	animate(context);
}

void UI::OnPlayersCreated(const Context& context, const PlayersGroup& players)
{
	if (!_data)
		return;

	_data->game = std::make_unique<Data::Game>(_window.getView(), players.GetCount() - 1);
	animate(context);
}

void UI::OnPlayerShowTrumpCard(const Context& context, const Player& player, const Card& card)
{
	if (!_data || !_data->game)
		return;

	auto& playerCards = _data->game->playerCards.GetCards(player.GetId());
	playerCards.ShowCard(card);
	animate(context);
}

void UI::OnStartGame(const Context& context)
{
	_data = std::make_unique<Data>();
	_data->flags |= Data::Flag::ClickToStart;
	animate(context);
}

void UI::OnUserWin(const Context& context, const Player& user)
{
	if (!_data)
		return;

	_data->flags |= Data::Flag::UserVictory;
	animate(context);
}

void UI::OnUserLose(const Context& context, const Player& opponent)
{
	if (!_data || !_data->game)
		return;

	_data->flags |= Data::Flag::UserDefeat;
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
	if (!_data || !_data->game)
		return;

	_data->game->roundCards.MoveFrom(card, _data->game->playerCards.GetCards(player.GetId()));
	animate(context);
}

sf::Vector2f UI::getDeckPosition() const
{
	const auto size = _window.getView().getSize();
	return { 0.9f * size.x, 0.5f * size.y };
}

void UI::animate(const Context& context)
{
	if (!_data)
		return;

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
	if (!NeedsToUpdate() || !_window.isOpen() || !_data)
		return;

	constexpr float interactOffset = 5.f;
	const auto size = _window.getView().getSize();
	sf::Cursor::Type cursorType = sf::Cursor::Arrow;

	{
		Screen::Table table;
		_window.draw(table);
	}

	bool finished = true;
	if (_data->flags & Data::Flag::ClickToStart)
	{
		Screen::Text text("click to start");
		text.setOrigin(0.5f * size);
		_window.draw(text);
		finished = false;
	}
	else if (_data->game)
	{
		{
			Screen::Deck deck(context.GetDeck());
			deck.setOrigin(getDeckPosition());
			_window.draw(deck);
		}

		auto& userCards = _data->game->playerCards.GetCards(context.GetPlayers().GetUser()->GetId());
		if (_data->flags & Data::Flag::UserPickingCard)
		{
			_data->userPick->result = {};
			const bool isUserAttacking = _data->userPick->options.attacking;
			const bool canSkip = !isUserAttacking || !_data->game->roundCards.IsEmpty();

			if (canSkip)
			{
				Screen::SkipButton skipButton;
				const sf::Vector2f center(0.5f * size.x, size.y - 1.5f * Screen::Card{}.getSize().y);
				skipButton.setOrigin(center);
				_window.draw(skipButton);

				if (::isPointInRectange(center, Screen::SkipButton::Size, Screen::SkipButton::Size, _data->cursorPosition, interactOffset))
				{
					_data->userPick->result.emplace();
					cursorType = sf::Cursor::Hand;
				}
			}

			if (auto pick = userCards.Pick(_data->cursorPosition, interactOffset, _data->userPick->options.filter))
			{
				_data->userPick->result.emplace(std::move(pick));
				cursorType = sf::Cursor::Hand;
			}
		}
		userCards.Hover(_data->userPick && _data->userPick->result && _data->userPick->result->card ? _data->userPick->result->card : std::nullopt);

		if (_data->arrow)
		{
			Screen::Arrow arrow(_data->arrow->direction);
			arrow.setOrigin(0.5f * size);
			_window.draw(arrow);
		}

		finished = _data->game->playerCards.Draw(delta, _window) && finished;
		finished = _data->game->roundCards.Draw(delta, _window) && finished;
	}

	if (_data->flags & Data::Flag::UserVictory)
	{
		Screen::Text text("victory");
		text.setOrigin(0.5f * size);
		_window.draw(text);
		finished = false;
	}
	else if (_data->flags & Data::Flag::UserDefeat)
	{
		Screen::Text text("defeat");
		text.setOrigin(0.5f * size);
		_window.draw(text);
		finished = false;
	}

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