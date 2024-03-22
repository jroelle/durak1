#include "UI.h"
#include <SFML/Graphics/RenderWindow.hpp>

UI::UI(sf::RenderWindow& window)
	: _window(window)
{
}

void UI::Update()
{
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
