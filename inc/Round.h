#pragma once
#include "IObserver.h"
#include <memory>

class Context;

class Round : public Observed
{
public:
	Round() = delete;
	Round(const Round&) = delete;

	Round(std::shared_ptr<Context>);
	Round(Round&&) = default;

	std::unique_ptr<Round> Run();
	std::shared_ptr<Context> GetContext() const;

private:
	std::shared_ptr<Context> _context;
};