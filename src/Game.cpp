#include "Game.h"
#include <thread>
#include <SFML/Graphics.hpp>
#include "Context.h"
#include "Round.h"
#include "UI.h"

namespace
{
	void renderingThread(std::shared_ptr<UI> ui)
	{
		ui->GetWindow().setVerticalSyncEnabled(true);
		ui->GetWindow().setActive(true);

		while (ui->GetWindow().isOpen())
		{
			ui->GetWindow().clear();
			ui->Update();
			ui->GetWindow().display();
		}
	}
}

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
	window.setActive(false);

	auto ui = std::make_shared<UI>(window);
	std::thread render(&renderingThread, ui);

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
			round = round->Run();
		}

		//window.clear();
		//ui->Update();
		//window.display();
	}
}