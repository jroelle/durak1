#pragma once
#include <vector>
#include <memory>
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
		void draw(sf::RenderTarget&, sf::RenderStates) const override final;
		void addChild(const std::shared_ptr<Drawing>&);

	protected:
		virtual void run(sf::RenderTarget&) const {}

	private:
		std::vector<std::shared_ptr<Drawing>> _children;
	};

	class Table : public Drawing
	{
	protected:
		void run(sf::RenderTarget&) const override;
	};

	class Card : public Drawing
	{
	public:
		virtual ~Card() = default;

		static constexpr float Width = 65.;
		static constexpr float Height = 100.;
		static constexpr float Bevel = Width * 0.075f;
	};

	class OpenCard final : public Card
	{
	public:
		OpenCard(const ::Card&);

	private:
		void run(sf::RenderTarget&) const override;

	private:
		::Card _card;
		static sf::Font _font;
	};

	class CloseCard final : public Card
	{
	private:
		void run(sf::RenderTarget&) const override;
	};

	class Suit final : public Drawing
	{
	private:
		void run(sf::RenderTarget&) const override;
	};

	class SkipButton final : public Drawing
	{
	public:
		static constexpr float Size = 25.;
		static constexpr float IconSize = 20.;

	private:
		void run(sf::RenderTarget&) const override;
	};

	class Deck final : public Drawing
	{
	public:
		Deck(const ::Deck&);

	private:
		void run(sf::RenderTarget&) const override;

	private:
		const ::Deck& _deck;
	};
}