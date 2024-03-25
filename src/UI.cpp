#include "UI.h"
#include <array>
#include <SFML/Graphics/RenderWindow.hpp>
#include "Drawing.h"
#include "Color.h"
#include "Hand.h"
#include "Card.h"
#include "Mutex.h"
#include "Context.h"

namespace
{
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

	class Animation
	{
	public:
		void Add()
		{
			// TODO
		}

		void Update(sf::RenderTarget&, double msDelta)
		{
			// TODO
		}

		bool IsEmpty() const
		{
			return true;
		}
	};
}

struct UI::Data
{
	struct Flag
	{
		enum Value : int
		{
			Default			= 0,
			DraggingCard	= 1 << 0,
			PickingCard		= 1 << 1,
			NeedRedraw		= 1 << 2,
		};
	};

	Animation animation;
	std::vector<Card> roundCards;
	std::underlying_type_t<Flag::Value> flags = Flag::Default;
	sf::Vector2f cursorPosition;
};

UI::UI(const std::string& title, unsigned int width, unsigned int height)
	: _window(sf::VideoMode{ width, height }, sf::String(title))
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

void UI::Update(const Context& context, double msDelta)
{
	if (!NeedsToUpdate())
		return;

	Mutex::Guard guard(Mutex::Get());

	const auto size = _window.getView().getSize();
	_data->animation.Update(_window, msDelta);

	{
		Screen::Table table;
		_window.draw(table);
	}

	{
		Screen::Deck deck(context.GetDeck());
		deck.setOrigin(size.x - 0.5f * Screen::Card::Width, 0.5f * size.y);
		_window.draw(deck);
	}

	if (_data->flags & Data::Flag::PickingCard)
	{
		Screen::SkipButton skipButton;
		skipButton.setOrigin(0.5f * size.x, size.y - Screen::Card::Height);
		_window.draw(skipButton);
	}

	if (_data->flags & Data::Flag::DraggingCard)
	{
		Screen::OpenCard openCard({ Card::Suit::Diamonds, Card::Rank::Jack });
		openCard.setOrigin(_data->cursorPosition);
		_window.draw(openCard);
	}

	if (_data->animation.IsEmpty())
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
			//_data->cursorPosition = toModel({ event.mouseMove.x, event.mouseMove.y });
			_data->flags |= Data::Flag::DraggingCard;
			_data->flags |= Data::Flag::NeedRedraw;
		}
		break;

	case sf::Event::EventType::MouseButtonReleased:
		_data->flags &= ~Data::Flag::DraggingCard;
		_data->flags |= Data::Flag::NeedRedraw;
		break;

	case sf::Event::EventType::MouseMoved:
		_data->cursorPosition = toModel({ event.mouseMove.x, event.mouseMove.y });
		_data->flags |= Data::Flag::NeedRedraw;
		break;
	}

	return false;
}

void UI::OnRoundStart(const Round& round)
{
	_data->flags |= Data::Flag::NeedRedraw;
}

void UI::OnPlayerAttack(const Player& attacker, const Card& attackCard)
{
	_data->flags |= Data::Flag::NeedRedraw;
}

void UI::OnPlayerDefend(const Player& defender, const Card& attackCard, const Card& defendCard)
{
	_data->flags |= Data::Flag::NeedRedraw;
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