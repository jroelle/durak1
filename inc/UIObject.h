#pragma once
#include "Card.h"
#include <SFML/Graphics/Rect.hpp>

namespace sf
{
	class Drawable;
	class Transformable;
	class Font;
}
class UIPainter;


class UIObject
{
public:
	virtual ~UIObject() = default;
	virtual void Draw(UIPainter&) const = 0;
	virtual sf::FloatRect GetBoundingRect() const = 0;
};

class UICard : public UIObject
{
public:
	virtual ~UICard() = default;
	void Draw(UIPainter&) const override final;
	sf::FloatRect GetBoundingRect() const override final;

	static constexpr float Width = 150.;
	static constexpr float Height = 215.;
	static constexpr float Bevel = Width * 0.075f;

protected:
	virtual void draw(UIPainter&) const = 0;
};

class UIOpenedCard final : public UICard
{
public:
	UIOpenedCard(const Card&);

private:
	void draw(UIPainter&) const override;

private:
	Card _card;
	static sf::Font _font;
};

class UIClosedCard final : public UICard
{
public:
	UIClosedCard() = default;

private:
	void draw(UIPainter&) const override;
};

class UISkipButton final : public UIObject
{
public:
	static constexpr float Size = 50.;
	static constexpr float IconSize = 40.;
	static constexpr float LineWidth = 5.;

	void Draw(UIPainter&) const override;
	sf::FloatRect GetBoundingRect() const override;
};