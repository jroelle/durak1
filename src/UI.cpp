#include "UI.h"
#include <queue>
#include <set>
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
		State speedMs = State{ sf::Vector2f(0.003f, 0.003f), 10.f };
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
				{
					if (_animations.front().onFinish)
						_animations.front().onFinish();
					_animations.pop();
				}

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
		using ForEachCard = std::function<void(const ::Card&)>;

		VisibleCards(const sf::View& view)
			: _view(view)
		{}

		virtual ~VisibleCards() = default;

		void MoveFrom(const ::Card& cardInfo, VisibleCards& other)
		{
			if (auto visibleCard = other.Remove(cardInfo))
				Add(std::move(visibleCard));
		}

		void Add(std::unique_ptr<VisibleCard>&& visibleCard)
		{
			if (auto* newCard = _list.push_back(std::move(visibleCard)))
			{
				Animation animation;
				animation.finalState = getNewCardState();
				newCard->StartAnimation(animation);
				onCardAdded(*newCard);
			}
		}

		std::unique_ptr<VisibleCard> Remove(const ::Card& cardInfo)
		{
			auto visibleCard = _list.remove(_list.find(cardInfo));
			if (visibleCard)
				onCardRemoved(*visibleCard);
			return visibleCard;
		}

		void StartAnimation(const Animation& animation)
		{
			_list.for_each([&animation](VisibleCard* visibleCard)
				{
					visibleCard->StartAnimation(animation);
					return false;
				});
		}

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

		std::optional<State> GetState(const ::Card& cardInfo) const
		{
			if (const auto* visibleCard = _list.find(cardInfo))
				return visibleCard->GetState();
			return {};
		}

	private:
		virtual void onCardAdded(const VisibleCard&) {}
		virtual void onCardRemoved(const VisibleCard&) {}
		virtual State getNewCardState() const = 0;

	protected:
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
		State getNewCardState() const override
		{
			const size_t roundCardCount = _list.size();
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
			const sf::Vector2f roundArea = 0.8f * _view.getSize();
			const sf::Vector2f gap = { (roundArea.x - columns * cardPairSize.x) / (columns - 1), (roundArea.y - rows * cardPairSize.y) / (rows - 1) };

			sf::Vector2f position;
			position.x = column * cardPairSize.x + std::max(column - 1, (size_t)0) * gap.x;
			position.y = row * cardPairSize.y + std::max(row - 1, (size_t)0) * gap.y;
			position += 0.f * cardSize;
			if (isOverlayed)
				position += overlapOffset;

			return { position, 0.f };
		}
	};

	class PlayerCards final : public VisibleCards
	{
	public:
		PlayerCards(const sf::View& view, const sf::Vector2f& position, const sf::Vector2f& faceDirection)
			: VisibleCards(view)
			, _position(position)
			, _faceDirection(faceDirection)
		{}

	private:
		void onCardAdded(const VisibleCard& card) override
		{

		}

		void onCardRemoved(const VisibleCard& card) override
		{

		}

		State getNewCardState() const override
		{

		}

	private:
		sf::Vector2f _position;
		sf::Vector2f _faceDirection;
	};

	class Players
	{
	public:
		Players(const sf::View& view, size_t botsNumber)
		{
			_players.reserve(botsNumber + 1);
			_players.emplace_back(view, sf::Vector2f{ 0.5f * view.getSize().x, view.getSize().y }, sf::Vector2f{ 0.f, -1.f });

			switch (botsNumber)
			{
			case 2:
				_players.emplace_back(view, sf::Vector2f{ 0.f, 0.5f * view.getSize().y }, sf::Vector2f{ 1.f, 0.f });
				[[fallthrough]];

			case 1:
				_players.emplace_back(view, sf::Vector2f{ 0.5f * view.getSize().x, 0.f }, sf::Vector2f{ 0.f, 1.f });
				break;
			}
			// TODO
		}

		size_t GetCount() const
		{
			return _players.size();
		}

		PlayerCards& GetCards(Player::Id id)
		{
			return _players[id];
		}

		const PlayerCards& GetCards(Player::Id id) const
		{
			return _players[id];
		}

		bool Draw(sf::Int32 msDelta, sf::RenderTarget& target)
		{
			bool res = true;
			for (auto& player : _players)
				res = player.Draw(msDelta, target) && res;
			return res;
		}

	private:
		using Index = ::Player::Id;
		std::vector<PlayerCards> _players;
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

void UI::Update(const Context& context, sf::Int32 msDelta)
{
	if (!_data)
		_data = std::make_unique<Data>(_window.getView(), context.GetPlayers().GetCount());

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
	finished = _data->playerCards.Draw(msDelta, _window) && finished;
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
	_data->flags |= Data::Flag::NeedRedraw;

	PlayerCards& playerCards = _data->playerCards.GetCards(player.GetId());
	const sf::Vector2f startPosition = getDeckPosition();
	for (const Card& cardInfo : cards)
	{
		auto visibleCard = std::make_unique<VisibleCard>(cardInfo, State{ startPosition, 0.f });
		playerCards.Add(std::move(visibleCard));
	}
}

void UI::OnPlayerDrawRoundCards(const Player& player, const std::vector<Card>& cards)
{
	_data->flags |= Data::Flag::NeedRedraw;

	PlayerCards& playerCards = _data->playerCards.GetCards(player.GetId());
	for (const auto& cardInfo : cards)
	{
		playerCards.MoveFrom(cardInfo, _data->roundCards);
	}
}

void UI::OnRoundStart(const Round& round)
{
	//_data->flags |= Data::Flag::NeedRedraw;
}

void UI::OnRoundEnd(const Round& round)
{
	_data->flags |= Data::Flag::NeedRedraw;
	_data->roundCards.RemoveAll();
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
	_data->flags |= Data::Flag::NeedRedraw;
	_data->roundCards.MoveFrom(card, _data->playerCards.GetCards(player.GetId()));
}

sf::Vector2f UI::getDeckPosition() const
{
	const auto size = _window.getView().getSize();
	return { 0.9f * size.x, 0.5f * size.y };
}