#pragma once
#include <memory>
#include <SFML/Graphics.hpp>
#include "IObserver.h"

namespace sf
{
	class RenderWindow;
	class RenderTarget;
	class Drawable;
}

class UI : public IObserver
{
public:
	UI(const std::string&, unsigned int width, unsigned int height);
	~UI();

	sf::RenderWindow& GetWindow();
	const sf::RenderWindow& GetWindow() const;
	void Update();

	void OnRoundUpdate(const Round&) override;
	void OnStartGame(const PlayersGroup&) override;
	void OnUserWin(const Player& user, const PlayersGroup&) override;
	void OnUserLose(const Player& opponent) override;

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