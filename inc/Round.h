#pragma once
#include <memory>
#include <unordered_map>

class Context;

class Round
{
public:
	using Cards = std::unordered_map<Card, std::optional<Card>>;
	using ActionCallback = std::function<void(const Cards&)>;

	Round(std::shared_ptr<Context>);
	std::unique_ptr<Round> Run();

private:
	std::shared_ptr<Context> _context;
	Cards _cards;
};