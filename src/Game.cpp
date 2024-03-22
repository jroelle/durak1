#include "Game.h"
#include "Context.h"
#include "Round.h"

Game::Game()
	: _context(std::make_shared<Context>(2))
{
}

std::shared_ptr<Context> Game::GetContext() const
{
	return _context;
}

void Game::Run()
{
	auto round = std::make_unique<Round>(GetContext());
	while (round)
	{
		// TODO
		// ...

		round = round->Run();
	}
}