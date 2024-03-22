#pragma once
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

	void Update();

	void OnRoundUpdate(const Round&) override;
	void OnStartGame(const PlayersGroup&) override;
	void OnUserWin(const Player& user, const PlayersGroup&) override;
	void OnUserLose(const Player& opponent) override;

private:
	sf::RenderWindow& _window;
};