#pragma once
#include <memory>
#include <optional>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Cursor.hpp>
#include "IController.h"
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
struct Settings;

class UI final : public IController
{
public:
	UI(const std::string&, unsigned int width, unsigned int height);
	~UI();

	sf::RenderWindow& GetWindow();
	const sf::RenderWindow& GetWindow() const;
	bool NeedsToUpdate() const;
	bool HandleEvent(const sf::Event&);
	void Pick(const Context&, std::shared_ptr<UserPick>) override;
	std::optional<Card> UserPickCard(const Context&, bool attacking, const PickCardFilter&) override;
	void SetSettings(const Context&, Settings&) override;

	void OnPlayerAttack(const Context&, const Player&, const Card&);
	void OnPlayerDefend(const Context&, const Player&, const Card&);
	void OnPlayerDrawDeckCards(const Context&, const Player&, const std::vector<Card>&);
	void OnPlayerDrawRoundCards(const Context&, const Player&, const std::vector<Card>&);
	void OnRoundStart(const Context&, const Round&);
	void OnRoundEnd(const Context&, const Round&);
	void OnPlayersCreated(const Context&, const PlayersGroup&);
	void OnPlayerShowTrumpCard(const Context&, const Player&, const Card&);
	void OnStartGame(const Context&, Settings&);
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