#include "Game.h"
#include "Context.h"
#include "Round.h"

Game::Game()
	: _context(2)
{
}

std::shader_ptr<Context> Game::GetContext() const
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