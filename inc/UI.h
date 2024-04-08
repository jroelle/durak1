#pragma once
#include <memory>
#include <optional>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Cursor.hpp>
#include "Card.h"

namespace sf
{
	class RenderWindow;
	class Event;
}
class Round;
class Player;
class Deck;
class Context;
class User;
class PlayersGroup;

class UI final
{
public:
	UI(const std::string&, unsigned int width, unsigned int height);
	~UI();

	sf::RenderWindow& GetWindow();
	const sf::RenderWindow& GetWindow() const;
	bool NeedsToUpdate() const;
	bool HandleEvent(const sf::Event&);
	bool IsLocked() const;
	std::optional<Card> UserPickCard(const User&);

	void OnPlayerAttack(const Context&, const Player&, const Card&);
	void OnPlayerDefend(const Context&, const Player&, const Card&);
	void OnPlayerDrawDeckCards(const Context&, const Player&, const std::vector<Card>&);
	void OnPlayerDrawRoundCards(const Context&, const Player&, const std::vector<Card>&);
	void OnRoundStart(const Context&, const Round&);
	void OnRoundEnd(const Context&, const Round&);
	void OnPlayersCreated(const Context&, const PlayersGroup&);
	void OnStartGame(const Context&, const Player& first);
	void OnUserWin(const Context&, const Player& user);
	void OnUserLose(const Context&, const Player& opponent);

private:
	sf::Vector2f toModel(const sf::Vector2i&) const;
	sf::Vector2i toScreen(const sf::Vector2f&) const;

	void onPlayerPlaceCard(const Context&, const Player&, const Card&);
	sf::Vector2f getDeckPosition() const;
	void animate(const Context&);
	void update(const Context&, sf::Time delta);

private:
	struct Data;

	sf::RenderWindow _window;
	sf::Cursor::Type _cursorType = sf::Cursor::Arrow;
	std::unique_ptr<Data> _data;
};