#include "UI.h"
#include <array>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include "UIObject.h"
#include "Hand.h"
#include "Card.h"

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
		TransformPainter(sf::RenderTarget& target, const sf::Vector2f& position, float angleDeg)
			: UIPainter(target)
			, _position(position)
			, _angleDeg(angleDeg)
		{}
		virtual ~TransformPainter() = default;

		void Draw(const sf::Drawable& object) override
		{
			ViewRestorer restorer(_target);

			sf::View view;
			view.move(_position);
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
		float _angleDeg;
	};
}

struct UI::Data
{
	std::array<Card, Hand::MinCount * 2> roundCards;
	// TODO
};

UI::UI(const std::string& title, unsigned int width, unsigned int height)
	: _window(sf::VideoMode{ width, height }, sf::String(title))
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

void UI::Update()
{
	TransformPainter painter(_window, { 200., 200. }, 30.);
	
	UIOpenedCard card1(Card{ Card::Suit::Diamonds, Card::Rank::Ace });
	UIClosedCard card2;

	card1.Draw(painter);
	card2.Draw(painter);
}

bool UI::HandleEvent(const sf::Event& event)
{
	return false;
}

void UI::OnRoundStart(const Round& round)
{

}

void UI::OnPlayerAttack(const Player& attacker, const Card& attackCard)
{

}

void UI::OnPlayerDefend(const Player& defender, const Card& attackCard, const Card& defendCard)
{

}

void UI::OnRoundEnd(const Round& round)
{

}

void UI::OnPlayerDrawCards(const Player& player, const Deck& deck)
{

}

void UI::OnPlayerDrawCards(const Player& player, const Round& round)
{

}

void UI::OnStartGame(const Player& first, const Context& context)
{

}

void UI::OnUserWin(const Player& user, const Context& context)
{

}

void UI::OnUserLose(const Player& opponent, const Context& context)
{

}