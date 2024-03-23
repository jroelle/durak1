#pragma once
#include <forward_list>
#include <memory>
#include <functional>
#include <optional>

class Round;
class Context;
class Player;
class Deck;
class Card;

class IObserver
{
public:
	virtual ~IObserver() = default;

	virtual void OnRoundStart(const Round&) = 0;
	virtual void OnPlayerAttack(const Player& attacker, const Card& attackCard) = 0;
	virtual void OnPlayerDefend(const Player& defender, const Card& attackCard, const Card& defendCard) = 0;
	virtual void OnRoundEnd(const Round&) = 0;
	virtual void OnPlayerDrawCards(const Player&, const Deck&) = 0;
	virtual void OnPlayerDrawCards(const Player&, const Round&) = 0;

	virtual void OnStartGame(const Player& first, const Context&) = 0;
	virtual void OnUserWin(const Player& user, const Context&) = 0;
	virtual void OnUserLose(const Player& opponent, const Context&) = 0;
};

class Observed
{
public:
	Observed() = default;
	Observed(const Observed&) = default;
	Observed(Observed&&) = default;
	virtual ~Observed() = default;

	void AddObserver(std::weak_ptr<IObserver> observer)
	{
		_list.push_front(std::move(observer));
	}

	using EventCallback = std::function<void(IObserver&)>;
	void HandleEvent(const EventCallback& callback) const
	{
		for (const auto& observer : _list)
		{
			auto ptr = observer.lock();
			callback(*ptr);
		}
	}

private:
	std::forward_list<std::weak_ptr<IObserver>> _list;
};