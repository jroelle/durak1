#include "Game.h"
#include <thread>
#include <mutex>
#include "Context.h"
#include "Round.h"
#include "UI.h"
#include "Utility.hpp"

namespace
{
	std::mutex g_renderMutex;

	inline void updateUI(std::shared_ptr<UI> ui)
	{
		ui->GetWindow().setVerticalSyncEnabled(true);
		ui->GetWindow().setActive(true);

		while (ui->GetWindow().isOpen())
		{
			auto& window = ui->GetWindow();
			for (auto event = sf::Event{}; window.pollEvent(event);)
			{
				if (event.type == sf::Event::Closed)
				{
					window.close();
					break;
				}

				ui->HandleEvent(event);
			}

			window.clear();
			{
				std::lock_guard<std::mutex> guard(g_renderMutex);
				ui->Update();
			}
			window.display();
		}
	}

	inline Player* findFirstPlayer(const PlayersGroup& players, Card::Suit trumpSuit)
	{
		std::optional<std::pair<Player*, Card>> firstPlayer;

		players.ForEach([&firstPlayer, trumpSuit](Player* player)
			{
				const auto card = player->FindLowestTrumpCard(trumpSuit);
				if (card && (!firstPlayer || card->GetRank() < firstPlayer->second.GetRank()))
					firstPlayer.emplace(player, *card);

				return firstPlayer && firstPlayer->second.GetRank() == Card::Rank::Min;
			});

		return firstPlayer ? firstPlayer->first : players.GetUser();
	}
}

Game::Game()
	: _context(std::make_shared<Context>(2))
{
}

void Game::Run()
{
	auto ui = std::make_shared<UI>("durak", 500, 500);

	ui->GetWindow().setActive(false);

	std::thread render(&updateUI, ui);

	Player* firstPlayer = findFirstPlayer(_context->GetPlayers(), _context->GetTrumpSuit());
	ui->OnStartGame(*firstPlayer, *_context);

	auto round = std::make_unique<Round>(_context, firstPlayer);
	round->AddObserver(ui);
	while (round)
	{
		round = round->Run();
	}
}