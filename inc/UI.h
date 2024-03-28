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

class UI
{
public:
	UI(const std::string&, unsigned int width, unsigned int height);
	~UI();

	sf::RenderWindow& GetWindow();
	const sf::RenderWindow& GetWindow() const;
	bool NeedsToUpdate() const;
	void Update(const Context&, sf::Int32 msDelta);
	bool HandleEvent(const sf::Event&);
	bool IsLocked() const;

	void OnRoundStart(const Round&);
	void OnPlayerAttack(const Player& attacker, const Card& attackCard);
	void OnPlayerDefend(const Player& defender, const Card& attackCard, const Card& defendCard);
	void OnRoundEnd(const Round&);
	void OnPlayerDrawCards(const Player&, const Deck&);
	void OnPlayerDrawCards(const Player&, const Round&);

	void OnStartGame(const Player& first, const Context&);
	void OnUserWin(const Player& user, const Context&);
	void OnUserLose(const Player& opponent, const Context&);

	std::optional<Card> UserPickCard(const User&);

private:
	sf::Vector2f toModel(const sf::Vector2i&) const;
	sf::Vector2i toScreen(const sf::Vector2f&) const;

	sf::Vector2f findRoundCardPlace(const std::optional<Card>& attackCard = {}) const;
	void onPlayerPlaceCard(const Player& player, const Card& attackCard, const std::optional<Card>& defendCard = {});

private:
	struct Data;

	sf::RenderWindow _window;
	sf::Cursor::Type _cursorType = sf::Cursor::Arrow;
	std::unique_ptr<Data> _data;
};