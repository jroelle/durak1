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

	protected:
		virtual void run(sf::RenderTarget&) const {}
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

		static constexpr unsigned int Size = 75;
		sf::Vector2f getSize() const;
		const sf::Font& getFont() const;
	};

	class OpenCard final : public Card
	{
	public:
		OpenCard(const ::Card&);

	private:
		void run(sf::RenderTarget&) const override;

	private:
		::Card _card;
	};

	class CloseCard final : public Card
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

	class Arrow final : public Drawing
	{
	public:
		static constexpr float Size = 25.f;

		Arrow(const sf::Vector2f& start, const sf::Vector2f& direction);

	private:
		void run(sf::RenderTarget&) const override;

	private:
		sf::Vector2f _start;
		sf::Vector2f _direction;
	};
}