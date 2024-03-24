#pragma once
#include <forward_list>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include "Card.h"

namespace sf
{
	class Font;
	class RenderTarget;
	class RenderStates;
}
class Deck;

namespace Screen
{
	class Drawing : public sf::Drawable, public sf::Transformable
	{
	public:
		virtual ~Drawing() = default;
		void draw(sf::RenderTarget&, sf::RenderStates) const override;
		void addChild(Drawing&&);

	private:
		std::forward_list<Drawing> _children;
	};

	class Table : public Drawing
	{
	protected:
		void draw(sf::RenderTarget&, sf::RenderStates) const override;
	};

	class Card : public Drawing
	{
	public:
		virtual ~Card() = default;

		static constexpr float Width = 150.;
		static constexpr float Height = 215.;
		static constexpr float Bevel = Width * 0.075f;
	};

	class OpenCard final : public Card
	{
	public:
		OpenCard(const ::Card&);

	private:
		void draw(sf::RenderTarget&, sf::RenderStates) const override;

	private:
		::Card _card;
		static sf::Font _font;
	};

	class CloseCard final : public Card
	{
	private:
		void draw(sf::RenderTarget&, sf::RenderStates) const override;
	};

	class Suit final : public Drawing
	{
	private:
		void draw(sf::RenderTarget&, sf::RenderStates) const override;
	};

	class SkipButton final : public Drawing
	{
	public:
		static constexpr float Size = 50.;
		static constexpr float IconSize = 40.;
		static constexpr float LineWidth = 5.;

	private:
		void draw(sf::RenderTarget&, sf::RenderStates) const override;
	};

	class Deck final : public Drawing
	{
	public:
		Deck(const ::Deck&);

	private:
		void draw(sf::RenderTarget&, sf::RenderStates) const override;

	private:
		const ::Deck& _deck;
	};
}