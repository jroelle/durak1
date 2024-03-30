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
	virtual void OnStartGame(const Player& first, const Context&) {}
	virtual void OnUserWin(const Player& user, const Context&) {}
	virtual void OnUserLose(const Player& opponent, const Context&) {}
};

class EventHandlers final : public EventHandler
{
public:
	using Variant = std::variant<std::weak_ptr<EventHandler>, EventHandler*>;

	static EventHandlers& Get()
	{
		static EventHandlers s_this;
		return s_this;
	}

	void Add(Variant variant)
	{
		_handlers.push_front(variant);
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
	void OnStartGame(const Player& first, const Context& context) override
	{
		forEach([&](EventHandler* handler) { handler->OnStartGame(first, context); });
	}
	void OnUserWin(const Player& user, const Context& context) override
	{
		forEach([&](EventHandler* handler) { handler->OnUserWin(user, context); });
	}
	void OnUserLose(const Player& opponent, const Context& context) override
	{
		forEach([&](EventHandler* handler) { handler->OnUserLose(opponent, context); });
	}

private:
	template<typename F>
	void forEach(const F& func)
	{
		for (auto& variant : _handlers)
		{
			switch (variant.index())
			{
			case 0:
				func(std::get<0>(variant).lock().get());
				break;

			case 1:
				func(std::get<1>(variant));
				break;
			}
		}
	}

private:
	static inline std::forward_list<Variant> _handlers;
};