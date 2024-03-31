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

	inline void UILoop(std::weak_ptr<Context> context)
	{
		constexpr unsigned int framerate = 60;
		if (auto ui = context.lock()->GetUI())
		{
			auto& window = ui->GetWindow();
			window.setFramerateLimit(framerate);
			window.setActive(true);
		}
		sf::Clock clock;
		while (!context.expired() && context.lock()->GetUI() && context.lock()->GetUI()->GetWindow().isOpen())
		{
			auto ctx = context.lock();
			auto ui = ctx ? ctx->GetUI() : nullptr;
			if (ui && ui->NeedsToUpdate())
			{
				ui->GetWindow().clear();
				ui->Update(*ctx, clock.restart().asMilliseconds());
				ui->GetWindow().display();
			}
			//std::this_thread::sleep_for(std::chrono::milliseconds(1000 / framerate));
		}
	}
	inline void logicLoop(std::shared_ptr<Context> context)
	{
		Player* firstPlayer = findFirstPlayer(context->GetPlayers(), context->GetTrumpSuit());
		EventHandlers::Get().OnStartGame(*firstPlayer, *context);
		auto round = std::make_unique<Round>(context, firstPlayer);
		while (round)
		{
			if (context->GetUI() && !context->GetUI()->IsLocked())
				round = round->Run();
		}
	}
}

void Game::Run()
{
	auto ui = std::make_shared<UI>("durak", 500, 500);
	EventHandlers::Get().Add(ui);

	auto context = std::make_shared<Context>(ui, 1);

	ui->GetWindow().setActive(false);
	std::thread render(&UILoop, context);
	render.detach();

	//std::thread logic(&logicLoop, context);
	//logic.detach();
	Player* firstPlayer = findFirstPlayer(context->GetPlayers(), context->GetTrumpSuit());
	EventHandlers::Get().OnStartGame(*firstPlayer, *context);
	auto round = std::make_unique<Round>(context, firstPlayer);

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

		if (!ui->IsLocked())
			round = round->Run();
	}
}