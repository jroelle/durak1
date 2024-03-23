#include "UI.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

class UI::Objects
{
public:

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
	sf::RectangleShape rect({ 50., 75. });
	rect.setFillColor(sf::Color::Blue);
	rect.setPosition(100., 200.);
	_window.draw(rect);
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
