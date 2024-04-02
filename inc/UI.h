#pragma once
#include <memory>
#include <optional>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Cursor.hpp>
#include "Event.hpp"
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

class UI : public EventHandler
{
public:
	UI(const std::string&, unsigned int width, unsigned int height);
	~UI();

	sf::RenderWindow& GetWindow();
	const sf::RenderWindow& GetWindow() const;
	bool NeedsToUpdate() const;
	void Update(const Context&, sf::Time delta);
	bool HandleEvent(const sf::Event&);
	bool IsLocked() const;
	std::optional<Card> UserPickCard(const User&);

public:
	void OnPlayerAttack(const Player&, const Card&) override;
	void OnPlayerDefend(const Player&, const Card&) override;
	void OnPlayerDrawDeckCards(const Player&, const std::vector<Card>&) override;
	void OnPlayerDrawRoundCards(const Player&, const std::vector<Card>&) override;
	void OnRoundStart(const Round&) override;
	void OnRoundEnd(const Round&) override;
	void OnPlayersCreated(const PlayersGroup&) override;
	void OnStartGame(const Player& first, const Context&) override;
	void OnUserWin(const Player& user, const Context&) override;
	void OnUserLose(const Player& opponent, const Context&) override;

private:
	sf::Vector2f toModel(const sf::Vector2i&) const;
	sf::Vector2i toScreen(const sf::Vector2f&) const;

	void onPlayerPlaceCard(const Player&, const Card&);
	sf::Vector2f getDeckPosition() const;
	void awaitUpdate();

private:
	struct Data;

	sf::RenderWindow _window;
	sf::Cursor::Type _cursorType = sf::Cursor::Arrow;
	std::unique_ptr<Data> _data;
};