#pragma once
#include "Card.h"

namespace sf
{
	class Drawable;
	class Transformable;
}
class UIPainter;


class UIObject
{
public:
	virtual ~UIObject() = default;
	virtual void Draw(UIPainter&) const = 0;
};

class UICard : public UIObject
{
public:
	virtual ~UICard() = default;
	void Draw(UIPainter&) const override final;

	static constexpr double Width = 71.;
	static constexpr double Height = 100.;
	static constexpr double InnerOffset = 5.;

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
};

class UIClosedCard final : public UICard
{
public:
	UIClosedCard() = default;

private:
	void draw(UIPainter&) const override;
};