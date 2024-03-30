#include "Game.h"
#include <thread>
#include <SFML/System/Clock.hpp>
#include "Context.h"
#include "Round.h"
#include "UI.h"
#include "Mutex.h"
#include "Event.hpp"

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
}

void Game::Run()
{
	auto ui = std::make_shared<UI>("durak", 500, 500);
	EventHandlers::Get().Add(ui);

	auto context = std::make_shared<Context>(ui, 2);

	auto& window = ui->GetWindow();
	window.setFramerateLimit(60);
	window.setActive(true);

	Player* firstPlayer = findFirstPlayer(context->GetPlayers(), context->GetTrumpSuit());
	EventHandlers::Get().OnStartGame(*firstPlayer, *context);

	auto round = std::make_unique<Round>(context, firstPlayer);
	sf::Clock clock;
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

		if (ui->NeedsToUpdate())
		{
			ui->GetWindow().clear();
			ui->Update(*context, clock.restart().asMilliseconds());
			ui->GetWindow().display();
		}
		else
		{
			round = round->Run();
		}
	}
}