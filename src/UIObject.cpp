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

	constexpr char getRank(Card::Rank rank)
	{
		switch (rank)
		{
		case Card::Rank::Number6:		return '6';
		case Card::Rank::Number7:		return '7';
		case Card::Rank::Number8:		return '8';
		case Card::Rank::Number9:		return '9';
		case Card::Rank::Jack:			return 'J';
		case Card::Rank::Queen:			return 'Q';
		case Card::Rank::King:			return 'K';
		case Card::Rank::Ace:			return 'A';
		}
		return {};
	}

	inline sf::VertexArray createBeveledRectangle(bool fill, const sf::Color& color, float width, float height, float bevel)
	{
		sf::VertexArray rect(fill ? sf::PrimitiveType::TriangleFan : sf::PrimitiveType::LineStrip, 9);
		rect[0].position = { -0.5f * width, 0.5f * height - bevel };
		rect[1].position = { -0.5f * width + bevel, 0.5f * height };
		rect[2].position = { 0.5f * width - bevel, 0.5f * height };
		rect[3].position = { 0.5f * width, 0.5f * height - bevel };
		rect[4].position = { 0.5f * width, -0.5f * height + bevel };
		rect[5].position = { 0.5f * width - bevel, -0.5f * height };
		rect[6].position = { -0.5f * width + bevel, -0.5f * height };
		rect[7].position = { -0.5f * width, -0.5f * height + bevel };
		rect[8].position = rect[0].position;
		setColor(rect, color);
		return rect;
	}

	inline void drawBeveledRectangle(UIPainter& painter, const sf::Color& fillColor, const sf::Color& outlineColor, float width, float height, float bevel)
	{
		auto card = createBeveledRectangle(true, fillColor, width, height, bevel);
		painter.Draw(card);

		if (fillColor != outlineColor)
		{
			card.setPrimitiveType(sf::PrimitiveType::LineStrip);
			setColor(card, outlineColor);
			painter.Draw(card);
		}
	}

	inline void drawPointPattern(UIPainter& painter, const sf::Color& color, float width, float height, float step)
	{
		sf::VertexArray points(sf::PrimitiveType::Points, 4);
		setColor(points, color);

		for (float x = 0.f; x < 0.5f * width; x += step)
		{
			for (float y = 0.f; y < 0.5f * height; y += step)
			{
				points[0].position = { x, y };
				points[1].position = { -x, y };
				points[2].position = { -x, -y };
				points[3].position = { x, -y };
				painter.Draw(points);
			}
		}
	}
}

void UICard::Draw(UIPainter& painter) const
{
	draw(painter);
}

sf::FloatRect UICard::GetBoundingRect() const
{
	return { -0.5f * Width, 0.5f * Height, Width, Height };
}

sf::Font UIOpenedCard::_font = loadFont("ds-kork3.ttf");

UIOpenedCard::UIOpenedCard(const Card& card)
	: _card(card)
{
}

void UIOpenedCard::draw(UIPainter& painter) const
{
	constexpr float textOffset = Width * 0.05f;

	drawBeveledRectangle(painter, Color::LightGrayGreen, Color::DarkBrown, Width, Height, Bevel);

	sf::Text text;
	text.setFont(_font);
	text.setCharacterSize(static_cast<unsigned int>(Width / 2.5));
	text.setPosition(-0.5f * Width + textOffset, -0.5f * Height + textOffset);
	text.setFillColor(Color::DarkBrown);

	text.setString(getRank(_card.GetRank()));
	painter.Draw(text);

	text.setPosition(-text.getPosition());
	text.rotate(180.f);
	painter.Draw(text);
}

void UIClosedCard::draw(UIPainter& painter) const
{
	constexpr float patternStepCoeff = 0.1f;
	constexpr float patternSizeCoeff = 0.95f;

	drawBeveledRectangle(painter, Color::DarkBrown, Color::LightGrayGreen, Width, Height, Bevel);
	drawPointPattern(painter, Color::LightGrayGreen, Width * patternSizeCoeff, Height * patternSizeCoeff, Width * patternStepCoeff);
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
	setColor(square, Color::LightGrayGreen);
	square[0].position = { -0.5f * Size, 0.5f * Size };
	square[1].position = { 0.5f * Size, 0.5f * Size };
	square[2].position = { 0.5f * Size, -0.5f * Size };
	square[3].position = { -0.5f * Size, -0.5f * Size };
	square[4].position = square[0].position;
	painter.Draw(square);

	sf::VertexArray cross(sf::PrimitiveType::Lines, 2);
	setColor(cross, Color::LightGrayGreen);
	cross[0].position = { -0.5f * IconSize, 0.5f * IconSize };
	cross[1].position = { 0.5f * IconSize, -0.5f * IconSize };
	painter.Draw(cross);

	cross[0].position = { 0.5f * IconSize, 0.5f * IconSize };
	cross[1].position = { -0.5f * IconSize, -0.5f * IconSize };
	painter.Draw(cross);
}

sf::FloatRect UISkipButton::GetBoundingRect() const
{
	return { -0.5f * Size, 0.5f * Size, Size, Size };
}