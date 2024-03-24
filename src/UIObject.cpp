#include "UIObject.h"
#include <cmath>
#include <numbers>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include "UI.h"
#include "Color.h"

namespace
{
	inline sf::Font loadFont(const std::string& fileName)
	{
		sf::Font font;
		font.loadFromFile(fileName);
		return font;
	}

	inline void setColor(sf::VertexArray& vertices, const sf::Color& color)
	{
		for (size_t i = 0; i < vertices.getVertexCount(); ++i)
			vertices[i].color = color;
	}
}

void UICard::Draw(UIPainter& painter) const
{
	drawOutline(painter);
	draw(painter);
}

sf::FloatRect UICard::GetBoundingRect() const
{
	return { -0.5f * Width, 0.5f * Height, Width, Height };
}

void UICard::drawOutline(UIPainter& painter) const
{
	sf::RectangleShape rect({ Width, Height });
	rect.setOrigin({ 0.5f * Width, 0.5f * Height });
	rect.setOutlineColor(Color::DarkBrown);
	rect.setFillColor(Color::LightGrayGreen);
	rect.setOutlineThickness(5.);
	painter.Draw(rect);
}

sf::Font UIOpenedCard::_font = loadFont("ds-kork3.ttf");

UIOpenedCard::UIOpenedCard(const Card& card)
	: _card(card)
{
}

void UIOpenedCard::draw(UIPainter& painter) const
{
	sf::RectangleShape rect({ Width - InnerOffset, Height - InnerOffset });
	sf::Text text;
	rect.setFillColor(sf::Color::Red);
	painter.Draw(rect);
}

void UIClosedCard::draw(UIPainter& painter) const
{
	sf::RectangleShape rect({ Width, Height });
	rect.setFillColor(Color::DarkBrown);
	painter.Draw(rect);
}

void UISkipButton::Draw(UIPainter& painter) const
{
	sf::RectangleShape rect;
	rect.setOrigin(Size * 0.5f, Size * 0.5f);
	rect.setSize({ Size, Size });
	rect.setFillColor(Color::DarkBrown);
	painter.Draw(rect);

	sf::Color bg = Color::LightGrayGreen;
	bg.a = 30;
	rect.setFillColor(bg);
	painter.Draw(rect);

	sf::VertexArray square(sf::PrimitiveType::LineStrip, 5);
	square[0].position = { -0.5f * Size, 0.5f * Size };
	square[1].position = { 0.5f * Size, 0.5f * Size };
	square[2].position = { 0.5f * Size, -0.5f * Size };
	square[3].position = { -0.5f * Size, -0.5f * Size };
	square[4].position = square[0].position;
	setColor(square, Color::LightGrayGreen);
	painter.Draw(square);

	sf::VertexArray cross(sf::PrimitiveType::Lines, 2);
	cross[0].position = { -0.5f * IconSize, 0.5f * IconSize };
	cross[1].position = { 0.5f * IconSize, -0.5f * IconSize };
	setColor(cross, Color::LightGrayGreen);
	painter.Draw(cross);

	cross[0].position = { 0.5f * IconSize, 0.5f * IconSize };
	cross[1].position = { -0.5f * IconSize, -0.5f * IconSize };
	painter.Draw(cross);
}

sf::FloatRect UISkipButton::GetBoundingRect() const
{
	return { -0.5f * Size, 0.5f * Size, Size, Size };
}