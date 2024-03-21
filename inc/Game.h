#pragma once
#include <memory>

class Context;

class Game
{
public:
	Game();
	std::shared_ptr<Context> GetContext() const;
	void Run();

private:
	std::shared_ptr<Context> _context;
};