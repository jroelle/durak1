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

	static constexpr double Width = 150.;
	static constexpr double Height = 215.;
	static constexpr double InnerOffset = 15.;

protected:
	virtual void draw(UIPainter&) const = 0;

private:
	void drawOutline(UIPainter&) const;
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
	static constexpr double Size = 50.;
	static constexpr double IconSize = 40.;
	static constexpr double LineWidth = 5.;

	void Draw(UIPainter&) const override;
	sf::FloatRect GetBoundingRect() const override;
};