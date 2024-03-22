#pragma once
#include <memory>
#include "IObserver.h"

namespace sf
{
	class RenderWindow;
}

class UI : public IObserver
{
public:
	UI() = delete;
	UI(sf::RenderWindow&);
	~UI();

	sf::RenderWindow& GetWindow() const;
	void Update();

	void OnRoundUpdate(const Round&) override;
	void OnStartGame(const PlayersGroup&) override;
	void OnUserWin(const Player& user, const PlayersGroup&) override;
	void OnUserLose(const Player& opponent) override;

private:
	class Objects;

	sf::RenderWindow& _window;
	std::unique_ptr<Objects> _objects;
};