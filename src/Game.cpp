#include "Game.h"
#include <mutex>
#include <thread>
#include <SFML/System/Clock.hpp>
#include "Context.h"
#include "Round.h"
#include "UI.h"
#include "Event.hpp"
#include "PlayersGroup.h"

namespace
{
	std::mutex g_mutex;

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

	class GameEventHandler final : public EventHandler
	{
	public:
		GameEventHandler(std::weak_ptr<Context> context)
			: _context(context)
		{
			EventHandlers::Get().Add(this);
		}

		void OnPlayerAttack(const Player& player, const Card& card)
		{
			callUI([&](UI& ui, const Context& context) { ui.OnPlayerAttack(context, player, card); });
		}
		void OnPlayerDefend(const Player& player, const Card& card)
		{
			callUI([&](UI& ui, const Context& context) { ui.OnPlayerDefend(context, player, card); });
		}
		void OnPlayerDrawDeckCards(const Player& player, const std::vector<Card>& cards)
		{
			callUI([&](UI& ui, const Context& context) { ui.OnPlayerDrawDeckCards(context, player, cards); });
		}
		void OnPlayerDrawRoundCards(const Player& player, const std::vector<Card>& cards)
		{
			callUI([&](UI& ui, const Context& context) { ui.OnPlayerDrawDeckCards(context, player, cards); });
		}
		void OnRoundStart(const Round& round)
		{
			callUI([&](UI& ui, const Context& context) { ui.OnRoundStart(context, round); });
		}
		void OnRoundEnd(const Round& round)
		{
			callUI([&](UI& ui, const Context& context) { ui.OnRoundEnd(context, round); });
		}
		void OnPlayersCreated(const PlayersGroup& players)
		{
			callUI([&](UI& ui, const Context& context) { ui.OnPlayersCreated(context, players); });
		}
		void OnStartGame(const Player& first)
		{
			callUI([&](UI& ui, const Context& context) { ui.OnStartGame(context, first); });
		}
		void OnUserWin(const Player& user)
		{
			callUI([&](UI& ui, const Context& context) { ui.OnUserWin(context, user); });
		}
		void OnUserLose(const Player& opponent)
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
		context->Setup(1);

		Player* firstPlayer = findFirstPlayer(context->GetPlayers(), context->GetTrumpSuit());
		if (!firstPlayer)
			return;

		EventHandlers::Get().OnStartGame(*firstPlayer);
		auto round = std::make_unique<Round>(context, *firstPlayer);
		while (round)
		{
			round = round->Run();
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
				break;
			}

			{
				std::lock_guard<std::mutex> guard(g_mutex);
				ui->HandleEvent(event);
			}
		}
	}
}