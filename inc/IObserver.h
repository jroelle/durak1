#pragma once
#include <list>
#include <memory>
#include <functional>

class Round;
class PlayersGroup;
class Player;

class IObserver
{
public:
	virtual ~IObserver() = default;

	virtual void OnRoundUpdate(const Round&) = 0;
	virtual void OnStartGame(const PlayersGroup&) = 0;
	virtual void OnUserWin(const Player& user, const PlayersGroup&) = 0;
	virtual void OnUserLose(const Player& opponent) = 0;
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
		_list.push_back(std::move(observer));
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
	std::list<std::weak_ptr<IObserver>> _list;
};