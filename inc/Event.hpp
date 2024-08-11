#pragma once
#include <forward_list>
#include <variant>
#include <memory>
#include <utility>

class Player;
class Deck;
class Round;
class Card;
class Context;
class PlayersGroup;
struct Settings;

class EventHandler
{
public:
	virtual ~EventHandler() = default;

	virtual void OnPlayerAttack(const Player&, const Card&) {}
	virtual void OnPlayerDefend(const Player&, const Card&) {}
	virtual void OnPlayerDrawDeckCards(const Player&, const std::vector<Card>&) {}
	virtual void OnPlayerDrawRoundCards(const Player&, const std::vector<Card>&) {}

	virtual void OnRoundStart(const Round&) {}
	virtual void OnRoundEnd(const Round&) {}

	virtual void OnPlayersCreated(const PlayersGroup&) {}
	virtual void OnPlayerShowTrumpCard(const Player&, const Card&) {}
	virtual void OnStartGame(Settings&) {}
	virtual void OnUserWin(const Player& user) {}
	virtual void OnUserLose(const Player& opponent) {}
};

class EventHandlers final : public EventHandler
{
public:
	static EventHandlers& Get()
	{
		static EventHandlers s_instance;
		return s_instance;
	}

	inline void Add(EventHandler* handler)
	{
		_handlers.push_front(handler);
	}

	inline void Remove(EventHandler* handler)
	{
		_handlers.remove(handler);
	}

public:
	void OnPlayerAttack(const Player& player, const Card& card) override
	{
		forEach([&](EventHandler* handler) { handler->OnPlayerAttack(player, card); });
	}
	void OnPlayerDefend(const Player& player, const Card& card) override
	{
		forEach([&](EventHandler* handler) { handler->OnPlayerDefend(player, card); });
	}
	void OnPlayerDrawDeckCards(const Player& player, const std::vector<Card>& cards) override
	{
		forEach([&](EventHandler* handler) { handler->OnPlayerDrawDeckCards(player, cards); });
	}
	void OnPlayerDrawRoundCards(const Player& player, const std::vector<Card>& cards) override
	{
		forEach([&](EventHandler* handler) { handler->OnPlayerDrawRoundCards(player, cards); });
	}
	void OnRoundStart(const Round& round) override
	{
		forEach([&](EventHandler* handler) { handler->OnRoundStart(round); });
	}
	void OnRoundEnd(const Round& round) override
	{
		forEach([&](EventHandler* handler) { handler->OnRoundEnd(round); });
	}
	void OnPlayersCreated(const PlayersGroup& players) override
	{
		forEach([&](EventHandler* handler) { handler->OnPlayersCreated(players); });
	}
	void OnPlayerShowTrumpCard(const Player& player, const Card& card) override
	{
		forEach([&](EventHandler* handler) { handler->OnPlayerShowTrumpCard(player, card); });
	}
	void OnStartGame(Settings& settings) override
	{
		forEach([&](EventHandler* handler) { handler->OnStartGame(settings); });
	}
	void OnUserWin(const Player& user) override
	{
		forEach([&](EventHandler* handler) { handler->OnUserWin(user); });
	}
	void OnUserLose(const Player& opponent) override
	{
		forEach([&](EventHandler* handler) { handler->OnUserLose(opponent); });
	}

private:
	template<typename F>
	void forEach(const F& func)
	{
		for (EventHandler* handler : _handlers)
			func(handler);
	}

private:
	std::forward_list<EventHandler*> _handlers;
};

class AutoEventHandler : private EventHandler
{
public:
	AutoEventHandler()
	{
		EventHandlers::Get().Add(this);
	}

	~AutoEventHandler()
	{
		EventHandlers::Get().Remove(this);
	}
};