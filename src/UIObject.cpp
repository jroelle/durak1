#include "UIObject.h"
#include <cmath>
#include <numbers>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Color.hpp>
#include "UI.h"

namespace
{
	struct Color
	{
		static inline sf::Color CardOutline = sf::Color::Blue;
		static inline sf::Color CardBg = sf::Color(230, 230, 230);
		static inline sf::Color ClosedCard = sf::Color::Blue;
		static inline sf::Color ButtonFill = sf::Color::Black;
		static inline sf::Color ButtonLine = sf::Color(128, 128, 128);
	};
}

void UICard::Draw(UIPainter& painter) const
{
	drawOutline(painter);
	draw(painter);
}

void UICard::drawOutline(UIPainter& painter) const
{
	sf::RectangleShape rect({ Width, Height });
	rect.setOutlineColor(Color::CardOutline);
	rect.setFillColor(Color::CardBg);
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
	rect.setFillColor(Color::ClosedCard);
	painter.Draw(rect);
}

void UISkipButton::Draw(UIPainter& painter) const
{
	sf::RectangleShape rect({ Size, Size });
	rect.setOrigin({ Size * 0.5f, Size * 0.5f });
	rect.setFillColor(Color::ButtonFill);
	rect.setOutlineColor(Color::ButtonLine);
	rect.setOutlineThickness(Size / 25);
	painter.Draw(rect);

	const double offset = IconSize * 0.5;
	sf::VertexArray line(sf::PrimitiveType::Lines, 2);

	line[0].color = Color::ButtonLine;
	line[1].color = Color::ButtonLine;

	line[0].position = { offset, -offset };
	line[1].position = { -offset, offset };
	painter.Draw(line);

	line[0].position = { -offset, -offset };
	line[1].position = { offset, offset };
	painter.Draw(line);
}