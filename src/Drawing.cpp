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
#include "Vector.h"

namespace
{
	inline sf::Font loadFont(const std::string& fileName)
	{
		sf::Font font;
		const bool result = font.loadFromFile(fileName);
		font.setSmooth(false);
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

	inline char getCardCharacter(const Card& card)
	{
		switch (card.GetSuit())
		{
		case Card::Suit::Hearts:
			switch (card.GetRank())
			{
			case Card::Rank::Number6:	return 'S';
			case Card::Rank::Number7:	return 'T';
			case Card::Rank::Number8:	return 'U';
			case Card::Rank::Number9:	return 'V';
			case Card::Rank::Number10:	return 'W';
			case Card::Rank::Jack:		return 'X';
			case Card::Rank::Queen:		return 'Y';
			case Card::Rank::King:		return 'Z';
			case Card::Rank::Ace:		return 'N';
			}
			break;

		case Card::Suit::Diamonds:
			switch (card.GetRank())
			{
			case Card::Rank::Number6:	return 'F';
			case Card::Rank::Number7:	return 'G';
			case Card::Rank::Number8:	return 'H';
			case Card::Rank::Number9:	return 'I';
			case Card::Rank::Number10:	return 'J';
			case Card::Rank::Jack:		return 'K';
			case Card::Rank::Queen:		return 'L';
			case Card::Rank::King:		return 'M';
			case Card::Rank::Ace:		return 'A';
			}
			break;

		case Card::Suit::Clubs:
			switch (card.GetRank())
			{
			case Card::Rank::Number6:	return 's';
			case Card::Rank::Number7:	return 't';
			case Card::Rank::Number8:	return 'u';
			case Card::Rank::Number9:	return 'v';
			case Card::Rank::Number10:	return 'w';
			case Card::Rank::Jack:		return 'x';
			case Card::Rank::Queen:		return 'y';
			case Card::Rank::King:		return 'z';
			case Card::Rank::Ace:		return 'n';
			}
			break;

		case Card::Suit::Spades:
			switch (card.GetRank())
			{
			case Card::Rank::Number6:	return 'f';
			case Card::Rank::Number7:	return 'g';
			case Card::Rank::Number8:	return 'h';
			case Card::Rank::Number9:	return 'i';
			case Card::Rank::Number10:	return 'j';
			case Card::Rank::Jack:		return 'k';
			case Card::Rank::Queen:		return 'l';
			case Card::Rank::King:		return 'm';
			case Card::Rank::Ace:		return 'a';
			}
			break;
		}
		return '?';
	}

	inline std::optional<sf::FloatRect> getGlyphBounds(const sf::Font& font, unsigned int size)
	{
		constexpr char glyphCode = '?';
		if (font.hasGlyph(glyphCode))
		{
			const auto& glyph = font.getGlyph(glyphCode, size, false);
			return glyph.bounds;
		}
		return std::nullopt;
	}

	inline sf::Vector2f getPixelSize(const sf::RenderTarget& target)
	{
		sf::Vector2f pixelSize = target.mapPixelToCoords({ 1, 1 }) - target.mapPixelToCoords({ 0, 0 });
		pixelSize.x = std::abs(pixelSize.x);
		pixelSize.y = std::abs(pixelSize.y);
		return pixelSize;
	}

	inline void rotateViewAt(sf::Vector2f coord, sf::View& view, float rotation)
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

	void Table::run(sf::RenderTarget& target) const
	{
		sf::RectangleShape table(target.getView().getSize());
		table.setFillColor(Color::DarkBrown);
		target.draw(table);
	}

	sf::Vector2f Card::getSize() const
	{
		const auto bounds = getGlyphBounds(getFont(), Size);
		return bounds ? bounds->getSize() : sf::Vector2f{};
	}

	const sf::Font& Card::getFont() const
	{
		static sf::Font font = loadFont("CARDS.TTF"); // https://www.dafont.com/playing-cards.charmap
		return font;
	}

	OpenCard::OpenCard(const ::Card& card)
		: _card(card)
	{
	}

	void OpenCard::run(sf::RenderTarget& target) const
	{
		auto bounds = getGlyphBounds(getFont(), Size);
		if (!bounds)
			return;

		const auto pixelSize = getPixelSize(target);
		bounds->width -= 2.f * pixelSize.x;
		bounds->height -= 2.f * pixelSize.y;

		sf::RectangleShape bg(bounds->getSize());
		bg.setOrigin(0.5f * bg.getSize());
		bg.setFillColor(Color::LightGrayGreen);
		target.draw(bg);

		sf::Text text;
		text.setFont(getFont());
		text.setCharacterSize(Size);
		text.setFillColor(Color::DarkBrown);
		text.setString(getCardCharacter(_card));
		text.setPosition(-0.61f * bounds->getSize().x, -0.24f * bounds->getSize().y);
		target.draw(text);
	}

	void CloseCard::run(sf::RenderTarget& target) const
	{
		constexpr float patternStepCoeff = 0.1f;
		constexpr float patternSizeCoeff = 0.85f;

		sf::RectangleShape card(getSize() - 2.f * getPixelSize(target));
		card.setOrigin(card.getSize() * 0.5f);
		card.setFillColor(Color::DarkBrown);
		card.setOutlineColor(Color::LightGrayGreen);
		card.setOutlineThickness(getPixelSize(target).x);
		target.draw(card);

		drawPointPattern(target, Color::LightGrayGreen, card.getSize().x * patternSizeCoeff, card.getSize().y * patternSizeCoeff, card.getSize().x * patternStepCoeff);
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
	}

	void Deck::run(sf::RenderTarget& target) const
	{
		if (auto lastCard = _deck.GetLast())
		{
			Screen::OpenCard openCard(*lastCard);
			openCard.setOrigin(0.f, -0.5f * openCard.getSize().x);

			Holder base(std::move(openCard));
			base.rotate(90.f);

			target.draw(base);
		}
		if (_deck.GetCount() > 1)
		{
			constexpr float deckHeightCoeff = 0.5f;
			const float deckHeight = static_cast<float>(_deck.GetCount()) * deckHeightCoeff;

			CloseCard firstCard;
			firstCard.setOrigin(0.f, -deckHeight);

			sf::RectangleShape deck({ firstCard.getSize().x, firstCard.getSize().y + deckHeight });
			deck.setOrigin(0.5f * deck.getSize());
			deck.setFillColor(Color::LightGrayGreen);
			target.draw(deck);
			target.draw(firstCard);
		}
	}

	Arrow::Arrow(const sf::Vector2f& direction)
		: _direction(direction)
	{
	}

	void Arrow::run(sf::RenderTarget& target) const
	{
		constexpr float arrowHeadOffset = Size * 0.2f;

		Holder<sf::VertexArray> holder(sf::PrimitiveType::Lines, 6);
		auto& vertices = holder.get();
		setColor(vertices, Color::LightGrayGreen);

		const sf::Vector2f top = { 0.f, -Size * 0.5f };

		vertices[0].position = { 0.f, Size * 1.5f };
		vertices[1].position = top;

		vertices[2].position = { -arrowHeadOffset, top.y + arrowHeadOffset };
		vertices[3].position = top;

		vertices[4].position = { arrowHeadOffset, top.y + arrowHeadOffset };
		vertices[5].position = top;

		holder.rotate(angleDegree({ 0.f, -1.f }, _direction));
		target.draw(holder);
	}

	Text::Text(const sf::String& string)
		: _string(string)
	{
	}

	void Text::set(const sf::String& string)
	{
		_string = string;
	}

	sf::FloatRect Text::getLocalBounds() const
	{
		return createText().getLocalBounds();
	}

	void Text::run(sf::RenderTarget& target) const
	{
		sf::Text text = createText();
		text.setFillColor(Color::LightGrayGreen);
		text.setOutlineColor(Color::DarkBrown);
		text.setOutlineThickness(1.f);
		target.draw(text);
	}

	const sf::Font& Text::getFont() const
	{
		static sf::Font font = loadFont("Flexi_IBM_VGA_False_0.ttf");
		return font;
	}

	sf::Text Text::createText() const
	{
		sf::Text text(_string, getFont(), CharacterSize);
		const auto size = text.getLocalBounds().getSize();
		text.setOrigin({ 0.5f * size.x, size.y });
		return text;
	}
}