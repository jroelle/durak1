#include "UI.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include "UIObject.h"

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

			_target.draw(object);
		}

	private:
		sf::Vector2f _position;
		float _angleDeg;
	};
}

struct UI::Data
{
	sf::Text textTemplate;
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

void UI::OnRoundUpdate(const Round& round)
{

}

void UI::OnStartGame(const PlayersGroup& players)
{

}

void UI::OnUserWin(const Player& user, const PlayersGroup& players)
{

}

void UI::OnUserLose(const Player& opponent)
{

}