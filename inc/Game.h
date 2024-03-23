#pragma once
#include <memory>

class Context;

class Game
{
public:
	Game();
	void Run();

private:
	std::shared_ptr<Context> _context;
};