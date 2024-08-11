#pragma once
#include <functional>
#include <optional>

class Card;
class Context;
struct Settings;

class IController
{
public:
	using PickCardFilter = std::function<bool(const Card&)>;

	class UserPick
	{
	public:
		virtual ~UserPick() = default;

		virtual bool Hover(sf::RenderTarget&, const sf::Vector2f& cursor) = 0;
		virtual void ResetResult() = 0;
		virtual bool HasResult() const = 0;
	};

	IController() = default;
	virtual ~IController() = default;
	virtual void Pick(const Context&, std::shared_ptr<UserPick>) = 0;
	virtual std::optional<Card> UserPickCard(const Context&, bool attacking, const PickCardFilter&) = 0;
	virtual void SetSettings(const Context&, Settings&) = 0;
};