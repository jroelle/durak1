#include "Drawing.h"
#include <cmath>
#include <numbers>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include "UI.h"
#include "Color.h"
#include "Card.h"
#include "Deck.h"

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

	inline void drawBeveledRectangle(sf::RenderTarget& target, const sf::Color& fillColor, const sf::Color& outlineColor, float width, float height, float bevel)
	{
		auto card = createBeveledRectangle(true, fillColor, width, height, bevel);
		target.draw(card);

		if (fillColor != outlineColor)
		{
			card.setPrimitiveType(sf::PrimitiveType::LineStrip);
			setColor(card, outlineColor);
			target.draw(card);
		}
	}

	inline void drawPointPattern(sf::RenderTarget& target, const sf::Color& color, float width, float height, float step)
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
				target.draw(points);
			}
		}
	}

	struct TransformView
	{
	public:
		TransformView(sf::RenderTarget& target, const sf::Transformable& drawing)
			: _target(target)
			, _view(_target.getView())
		{
			sf::View newView = _view;

			newView.setCenter(_view.getCenter() + drawing.getOrigin());
			newView.setRotation(_view.getRotation() + drawing.getRotation());

			_target.setView(newView);
		}

		~TransformView()
		{
			_target.setView(_view);
		}

	private:
		sf::RenderTarget& _target;
		const sf::View _view;
	};
}

namespace Screen
{
	void Drawing::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		TransformView transformView(target, *this);
		target.draw(*this, states);
		for (const Drawing& drawing : _children)
		{
			drawing.draw(target, states);
		}
	}

	void Drawing::addChild(Drawing&& child)
	{
		_children.insert_after(_children.end(), std::move(child));
	}

	void Table::draw(sf::RenderTarget& target, sf::RenderStates) const
	{
		sf::RectangleShape table(target.getView().getSize());
		table.setFillColor(Color::DarkBrown);
		target.draw(table);
	}

	sf::Font OpenCard::_font = loadFont("ds-kork3.ttf");

	OpenCard::OpenCard(const ::Card& card)
		: _card(card)
	{
	}

	void OpenCard::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		constexpr float textOffset = Width * 0.05f;

		drawBeveledRectangle(target, Color::LightGrayGreen, Color::DarkBrown, Width, Height, Bevel);

		sf::Text text;
		text.setFont(_font);
		text.setCharacterSize(static_cast<unsigned int>(Width / 2.5));
		text.setPosition(-0.5f * Width + textOffset, -0.5f * Height + textOffset);
		text.setFillColor(Color::DarkBrown);

		text.setString(getRank(_card.GetRank()));
		target.draw(text);

		text.setPosition(-text.getPosition());
		text.rotate(180.f);
		target.draw(text);
	}

	void CloseCard::draw(sf::RenderTarget& target, sf::RenderStates) const
	{
		constexpr float patternStepCoeff = 0.1f;
		constexpr float patternSizeCoeff = 0.95f;

		drawBeveledRectangle(target, Color::DarkBrown, Color::LightGrayGreen, Width, Height, Bevel);
		drawPointPattern(target, Color::LightGrayGreen, Width * patternSizeCoeff, Height * patternSizeCoeff, Width * patternStepCoeff);
	}

	void Suit::draw(sf::RenderTarget& target, sf::RenderStates) const
	{
		// TODO
	}

	void SkipButton::draw(sf::RenderTarget& target, sf::RenderStates) const
	{
		sf::RectangleShape rect;
		rect.setOrigin(Size * 0.5f, Size * 0.5f);
		rect.setSize({ Size, Size });
		rect.setFillColor(Color::DarkBrown);
		target.draw(rect);

		sf::Color bg = Color::LightGrayGreen;
		bg.a = 30;
		rect.setFillColor(bg);
		target.draw(rect);

		sf::VertexArray square(sf::PrimitiveType::LineStrip, 5);
		setColor(square, Color::LightGrayGreen);
		square[0].position = { -0.5f * Size, 0.5f * Size };
		square[1].position = { 0.5f * Size, 0.5f * Size };
		square[2].position = { 0.5f * Size, -0.5f * Size };
		square[3].position = { -0.5f * Size, -0.5f * Size };
		square[4].position = square[0].position;
		target.draw(square);

		sf::VertexArray cross(sf::PrimitiveType::Lines, 2);
		setColor(cross, Color::LightGrayGreen);
		cross[0].position = { -0.5f * IconSize, 0.5f * IconSize };
		cross[1].position = { 0.5f * IconSize, -0.5f * IconSize };
		target.draw(cross);

		cross[0].position = { 0.5f * IconSize, 0.5f * IconSize };
		cross[1].position = { -0.5f * IconSize, -0.5f * IconSize };
		target.draw(cross);
	}

	Deck::Deck(const ::Deck& deck)
		: _deck(deck)
	{
		if (auto lastCard = _deck.GetLast())
		{
			Screen::OpenCard openCard(*lastCard);
			addChild(std::move(openCard));
		}
		{
			Screen::CloseCard firstCard;
			addChild(std::move(firstCard));
		}
	}

	void Deck::draw(sf::RenderTarget& target, sf::RenderStates) const
	{
		// TODO
	}
}