#include "Game.h"
#include <thread>
#include "Context.h"
#include "Round.h"
#include "UI.h"
#include "Utility.hpp"

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
	auto ui = std::make_shared<UI>("durak", 500, 500);
	while (ui->GetWindow().isOpen())
	{
		auto& window = ui->GetWindow();
		for (auto event = sf::Event{}; window.pollEvent(event);)
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
		}

		auto round = std::make_unique<Round>(GetContext());
		round->AddObserver(ui);
		while (round)
		{
			round = round->Run();
		}

		window.clear();
		ui->Update();
		window.display();
	}
}