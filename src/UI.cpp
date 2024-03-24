#include "UI.h"
#include <array>
#include <SFML/Graphics/RenderWindow.hpp>
#include "UIObject.h"
#include "Color.h"
#include "Hand.h"
#include "Card.h"
#include "Mutex.h"

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

	class TransformPainter : public UIPainter
	{
	public:
		using UIPainter::UIPainter;
		virtual ~TransformPainter() = default;

		void Draw(const sf::Drawable& object) override
		{
			ViewRestorer restorer(_target);

			sf::View view;
			view.move(-_position);
			view.setRotation(_angleDeg);
			_target.setView(view);

			_target.draw(object);
		}

		void SetPosition(const sf::Vector2f& position)
		{
			_position = position;
		}

		const sf::Vector2f& GetPosition() const
		{
			return _position;
		}

		void SetAngleDeg(float angleDeg)
		{
			_angleDeg = angleDeg;
		}

		float GetAngleDeg() const
		{
			return _angleDeg;
		}

	private:
		sf::Vector2f _position;
		float _angleDeg = 0.;
	};

	class Animation
	{
	public:
		void Add()
		{
			// TODO
		}

		void Update(double msDelta)
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

void UI::Update(double msDelta)
{
	if (!NeedsToUpdate())
		return;

	Mutex::Guard guard(Mutex::Get());
	const auto size = _window.getView().getSize();

	sf::RectangleShape bg(size);
	bg.setFillColor(Color::DarkBrown);
	_window.draw(bg);

	TransformPainter painter(_window);

	if (_data->flags & Data::Flag::PickingCard)
	{
		UISkipButton button;
		painter.SetPosition({ size.x, size.y * 1.5f });
		button.Draw(painter);
	}

	_data->animation.Update(msDelta);

	if (_data->flags & Data::Flag::DraggingCard)
	{
		UIOpenedCard card(Card{ Card::Suit::Diamonds, Card::Rank::Ace });
		painter.SetPosition(_data->cursorPosition);
		card.Draw(painter);
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
	return _window.mapPixelToCoords(screen * 2);
}

sf::Vector2i UI::toScreen(const sf::Vector2f& model) const
{
	return _window.mapCoordsToPixel(model * 0.5f);
}