#include "Game.h"
#include <thread>
#include "Context.h"
#include "Round.h"
#include "UI.h"
#include "Utility.hpp"

namespace
{
	using PtrUI = std::shared_ptr<UI>;
	using AtomicUI = std::atomic<PtrUI>;

	template<typename T>
	inline utility::atomic_accessor<T> access(std::atomic<std::shared_ptr<T>>& atomic)
	{
		return utility::atomic_accessor<T>(atomic);
	}

	void renderingThread(AtomicUI& ui)
	{
		if (auto accessor = access(ui))
		{
			accessor.get()->GetWindow().setVerticalSyncEnabled(true);
			accessor.get()->GetWindow().setActive(true);
		}

		while (auto accessor = access(ui))
		{
			if (!accessor.get()->GetWindow().isOpen())
			{
				accessor.set_changed(false);
				break;
			}

			accessor.get()->GetWindow().clear();
			accessor.get()->Update();
			accessor.get()->GetWindow().display();
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
	auto ui = AtomicUI(std::make_shared<UI>("durak", 500, 500));
	if (auto accessor = access(ui))
		accessor.get()->GetWindow().setActive(false);

	std::thread render(&renderingThread, ui);

	while (auto accessor = access(ui))
	{
		auto& window = accessor.get()->GetWindow();
		if (!window.isOpen())
		{
			accessor.set_changed(false);
			break;
		}

		for (auto event = sf::Event{}; window.pollEvent(event);)
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
		}

		auto round = std::make_unique<Round>(GetContext());
		round->AddObserver(accessor.get());
		while (round)
		{
			round = round->Run();
		}

		//window.clear();
		//ui->Update();
		//window.display();
	}
}