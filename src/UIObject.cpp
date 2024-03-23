#include "UIObject.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include "UI.h"

void UICard::Draw(UIPainter& painter) const
{
	drawOutline(painter);
	draw(painter);
}

void UICard::drawOutline(UIPainter& painter) const
{
	sf::RectangleShape rect({ Width, Height });
	rect.setOutlineColor(sf::Color::Blue);
	rect.setFillColor(sf::Color::White);
	rect.setOutlineThickness(5.);
	painter.Draw(rect);
}

UIOpenedCard::UIOpenedCard(const Card& card)
	: _card(card)
{
}

void UIOpenedCard::draw(UIPainter& painter) const
{
	sf::RectangleShape rect({ Width - InnerOffset, Height - InnerOffset });
	rect.setFillColor(sf::Color::Red);
	painter.Draw(rect);
}

void UIClosedCard::draw(UIPainter& painter) const
{
	sf::RectangleShape rect({ Width, Height });
	rect.setFillColor(sf::Color::Blue);
	painter.Draw(rect);
}