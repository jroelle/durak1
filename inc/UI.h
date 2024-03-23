#pragma once
#include <memory>
#include <SFML/Graphics.hpp>
#include "IObserver.h"

namespace sf
{
	class RenderWindow;
	class RenderTarget;
	class Drawable;
	class Event;
}

class UI : public IObserver
{
public:
	UI(const std::string&, unsigned int width, unsigned int height);
	~UI();

	sf::RenderWindow& GetWindow();
	const sf::RenderWindow& GetWindow() const;
	void Update(double msDelta);
	bool HandleEvent(const sf::Event&);

	void OnRoundStart(const Round&) override;
	void OnPlayerAttack(const Player& attacker, const Card& attackCard) override;
	void OnPlayerDefend(const Player& defender, const Card& attackCard, const Card& defendCard) override;
	void OnRoundEnd(const Round&) override;
	void OnPlayerDrawCards(const Player&, const Deck&) override;
	void OnPlayerDrawCards(const Player&, const Round&) override;

	void OnStartGame(const Player& first, const Context&) override;
	void OnUserWin(const Player& user, const Context&) override;
	void OnUserLose(const Player& opponent, const Context&) override;

private:
	struct Data;

	sf::RenderWindow _window;
	std::unique_ptr<Data> _data;
};

class UIPainter
{
public:
	UIPainter(sf::RenderTarget& target)
		: _target(target)
	{}
	virtual ~UIPainter() = default;
	virtual void Draw(const sf::Drawable&) = 0;

protected:
	sf::RenderTarget& _target;
};