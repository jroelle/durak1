#include "Drawing.h"
#include <cmath>
#include <numbers>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include "UI.h"
#include "Color.h"
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

	void rotateViewAt(sf::Vector2f coord, sf::View& view, float rotation)
	{
		const sf::Vector2f offset{ coord - view.getCenter() };
		const float rotationInRadians{ rotation * std::numbers::pi_v<float> / 180.f };
		const float sine{ std::sin(rotationInRadians) };
		const float cosine{ std::cos(rotationInRadians) };
		const sf::Vector2f rotatedOffset{ cosine * offset.x - sine * offset.y, sine * offset.x + cosine * offset.y };
		view.rotate(rotation);
		view.move(offset - rotatedOffset);
	}

	struct ViewGuard
	{
	public:
		ViewGuard(sf::RenderTarget& target, const sf::Transformable& drawing)
			: _target(target)
			, _view(_target.getView())
		{
			sf::View newView = _view;

			newView.setCenter(_view.getCenter() - drawing.getOrigin());
			//newView.setRotation(_view.getRotation() - drawing.getRotation());
			rotateViewAt(drawing.getOrigin(), newView, drawing.getRotation());

			_target.setView(newView);
		}

		~ViewGuard()
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
	template<typename T>
	class Holder final : public Drawing
	{
	public:
		Holder(const T& drawing)
			: _drawing(drawing)
		{}

		Holder(T&& drawing)
			: _drawing(std::move(drawing))
		{}

		template<typename... Args>
		Holder(Args&&... args)
			: _drawing(std::forward<Args>(args)...)
		{}

		T& get()
		{
			return _drawing;
		}

		const T& get() const
		{
			return _drawing;
		}

	protected:
		void run(sf::RenderTarget& target) const override
		{
			target.draw(get());
		}

	private:
		T _drawing;
	};

	void Drawing::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		ViewGuard guard(target, *this);
		run(target);
		for (const auto& drawing : _children)
		{
			drawing->draw(target, states);
		}

		if constexpr (false)
		{
			sf::VertexArray axis(sf::PrimitiveType::LineStrip, 3);
			setColor(axis, sf::Color::Red);
			axis[0].position = { 0.f, 0.f };
			axis[1].position = { 20.f, 0.f };
			axis[2].position = { 18.f, 5.f };
			target.draw(axis);

			setColor(axis, sf::Color::Blue);
			axis[1].position = { 0.f, 20.f };
			axis[2].position = { -5.f, 18.f };
			target.draw(axis);
		}
	}

	Drawing& Drawing::addChild(const std::shared_ptr<Drawing>& child)
	{
		_children.push_back(child);
		return *_children.back();
	}

	void Table::run(sf::RenderTarget& target) const
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

	void OpenCard::run(sf::RenderTarget& target) const
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

	void CloseCard::run(sf::RenderTarget& target) const
	{
		constexpr float patternStepCoeff = 0.1f;
		constexpr float patternSizeCoeff = 0.95f;

		drawBeveledRectangle(target, Color::DarkBrown, Color::LightGrayGreen, Width, Height, Bevel);
		drawPointPattern(target, Color::LightGrayGreen, Width * patternSizeCoeff, Height * patternSizeCoeff, Width * patternStepCoeff);
	}

	void Suit::run(sf::RenderTarget& target) const
	{
		// TODO
	}

	void SkipButton::run(sf::RenderTarget& target) const
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
			auto child = std::make_shared<Screen::Drawing>();
			child->rotate(90.f);

			auto openCard = std::make_shared<Screen::OpenCard>(*lastCard);
			openCard->setOrigin(0.f, -0.5f * Card::Width);
			child->addChild(openCard);

			addChild(child);
		}
		if (_deck.GetCount() > 1)
		{
			constexpr float deckHeightCoeff = 1.f;
			const float deckHeight = static_cast<float>(_deck.GetCount());

			auto cards = createBeveledRectangle(true, Color::LightGrayGreen, Card::Width, Card::Height, Card::Bevel);
			addChild(std::make_shared<Holder<sf::VertexArray>>(std::move(cards)));

			cards = createBeveledRectangle(false, Color::LightGrayGreen, Card::Width, Card::Height, Card::Bevel);
			addChild(std::make_shared<Holder<sf::VertexArray>>(std::move(cards)));

			auto firstCard = std::make_shared<Screen::CloseCard>();
			firstCard->setOrigin(0.f, -deckHeight);
			addChild(firstCard);
		}
	}
}