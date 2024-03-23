#pragma once
#include <memory>
#include <vector>
#include "IObserver.h"
#include "Hand.h"

class Context;

class Round : public Observed
{
public:
	using Cards = std::vector<Card>;

	Round() = delete;
	Round(const Round&) = delete;

	Round(std::shared_ptr<Context>, Player* attacker);
	Round(Round&&) = default;

	std::unique_ptr<Round> Run();
	std::shared_ptr<Context> GetContext() const;
	const Cards& GetCards() const;

private:
	std::shared_ptr<Context> _context;
	Player* _attacker = nullptr;
	Cards _cards;
};