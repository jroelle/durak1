#include "Game.h"
#include <thread>
#include <SFML/System/Clock.hpp>
#include "Context.h"
#include "Round.h"
#include "UI.h"
#include "Event.hpp"
#include "PlayersGroup.h"

namespace
{
	inline Player* findFirstPlayer(const PlayersGroup& players, Card::Suit trumpSuit)
	{
		std::optional<std::pair<Player*, Card>> firstPlayer;

		players.ForEach([&firstPlayer, trumpSuit](Player* player)
			{
				const auto card = player->FindLowestTrumpCard(trumpSuit);
				if (card)
				{
					EventHandlers::Get().OnPlayerShowTrumpCard(*player, *card);

					if (!firstPlayer || card->GetRank() < firstPlayer->second.GetRank())
						firstPlayer.emplace(player, *card);
				}

				return firstPlayer && firstPlayer->second.GetRank() == Card::Rank::Min;
			});

		return firstPlayer ? firstPlayer->first : players.GetUser();
	}

	class GameEventHandler final : public EventHandler
	{
	public:
		GameEventHandler(std::weak_ptr<Context> context)
			: _context(context)
		{
			EventHandlers::Get().Add(this);
		}

		void OnPlayerAttack(const Player& player, const Card& card) override
		{
			callUI([&](UI& ui, const Context& context) { ui.OnPlayerAttack(context, player, card); });
		}
		void OnPlayerDefend(const Player& player, const Card& card) override
		{
			callUI([&](UI& ui, const Context& context) { ui.OnPlayerDefend(context, player, card); });
		}
		void OnPlayerDrawDeckCards(const Player& player, const std::vector<Card>& cards) override
		{
			callUI([&](UI& ui, const Context& context) { ui.OnPlayerDrawDeckCards(context, player, cards); });
		}
		void OnPlayerDrawRoundCards(const Player& player, const std::vector<Card>& cards) override
		{
			callUI([&](UI& ui, const Context& context) { ui.OnPlayerDrawRoundCards(context, player, cards); });
		}
		void OnRoundStart(const Round& round) override
		{
			callUI([&](UI& ui, const Context& context) { ui.OnRoundStart(context, round); });
		}
		void OnRoundEnd(const Round& round) override
		{
			callUI([&](UI& ui, const Context& context) { ui.OnRoundEnd(context, round); });
		}
		void OnPlayersCreated(const PlayersGroup& players) override
		{
			callUI([&](UI& ui, const Context& context) { ui.OnPlayersCreated(context, players); });
		}
		void OnPlayerShowTrumpCard(const Player& player, const Card& card) override
		{
			callUI([&](UI& ui, const Context& context) { ui.OnPlayerShowTrumpCard(context, player, card); });
		}
		void OnStartGame() override
		{
			callUI([&](UI& ui, const Context& context) { ui.OnStartGame(context); });
		}
		void OnUserWin(const Player& user) override
		{
			callUI([&](UI& ui, const Context& context) { ui.OnUserWin(context, user); });
		}
		void OnUserLose(const Player& opponent) override
		{
			callUI([&](UI& ui, const Context& context) { ui.OnUserLose(context, opponent); });
		}

	private:
		template<typename T>
		void callUI(const T& callback)
		{
			auto actualContext = _context.lock();
			if (auto ui = actualContext ? actualContext->GetUI() : nullptr)
				callback(*ui, *actualContext);
		}

	private:
		std::weak_ptr<Context> _context;
	};

	inline void gameLoop(std::shared_ptr<UI> ui)
	{
		{
			constexpr unsigned int framerate = 60;

			auto& window = ui->GetWindow();
			window.setActive(true);
			window.setFramerateLimit(framerate);
		}

		auto context = std::make_shared<Context>(ui);
		GameEventHandler gameEventHandler(context);
		EventHandlers::Get().OnStartGame();
		context->Setup(1);

		Player* firstPlayer = findFirstPlayer(context->GetPlayers(), context->GetTrumpSuit());
		if (!firstPlayer)
			return;

		auto round = std::make_unique<Round>(*firstPlayer, context->GetPlayers().GetDefender(*firstPlayer));
		while (round)
		{
			round = round->Run(*context);
		}
	}
}

void Game::Run()
{
	auto ui = std::make_shared<UI>("durak", 500, 500);
	ui->GetWindow().setActive(false);

	std::thread game(&gameLoop, ui);
	game.detach();

	while (ui->GetWindow().isOpen())
	{
		auto& window = ui->GetWindow();
		for (auto event = sf::Event{}; window.pollEvent(event);)
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
				return;
			}

			ui->HandleEvent(event);
		}
	}
}