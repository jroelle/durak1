#include "Game.h"
#include <SFML/Graphics.hpp>
#include "Context.h"
#include "Round.h"
#include "UI.h"

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
	sf::RenderWindow window(sf::VideoMode{ 500, 500 }, "durak");
	window.setVerticalSyncEnabled(true);

	auto ui = std::make_shared<UI>(window);
	while (window.isOpen())
	{
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
			// TODO
			// ...

			round = round->Run();
		}

		window.clear();
		ui->Update();
		window.display();
	}
}