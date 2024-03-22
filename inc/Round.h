#pragma once
#include <memory>

class Context;

class Round
{
public:
	Round(std::shared_ptr<Context>);
	std::unique_ptr<Round> Run();

private:
	std::shared_ptr<Context> _context;
};