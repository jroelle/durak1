#include "Game.h"
#include <thread>
#include <SFML/System/Clock.hpp>
#include "Context.h"
#include "Round.h"
#include "UI.h"
#include "Utility.hpp"
#include "Mutex.h"

namespace
{
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

	inline void UILoop(std::shared_ptr<Context> context)
	{
		constexpr unsigned int framerate = 60;

		if (auto ui = context->GetUI())
		{
			auto& window = ui->GetWindow();
			window.setFramerateLimit(framerate);
			window.setActive(true);
		}

		sf::Clock clock;
		while (context->GetUI() && context->GetUI()->GetWindow().isOpen())
		{
			auto ui = context->GetUI();
			if (ui && ui->NeedsToUpdate())
			{
				ui->GetWindow().clear();
				ui->Update(*context, clock.restart().asMilliseconds());
				ui->GetWindow().display();
			}
			//std::this_thread::sleep_for(std::chrono::milliseconds(1000 / framerate));
		}
	}

	inline void logicLoop(std::shared_ptr<Context> context)
	{
		Player* firstPlayer = findFirstPlayer(context->GetPlayers(), context->GetTrumpSuit());
		if (auto ui = context->GetUI())
			ui->OnStartGame(*firstPlayer, *context);

		auto round = std::make_unique<Round>(context, firstPlayer);

		while (round)
		{
			round = round->Run();
		}
	}
}

void Game::Run()
{
	auto ui = std::make_shared<UI>("durak", 500, 500);
	auto context = std::make_shared<Context>(ui, 2);

	ui->GetWindow().setActive(false);

	std::thread render(&UILoop, context);
	render.detach();

	std::thread logic(&logicLoop, context);
	logic.detach();

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
	}
}