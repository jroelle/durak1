#pragma once
#include <memory>

class Context;

class Game
{
public:
	Game();
	std::shader_ptr<Context> GetContext() const;
	void Run();

private:
	std::shader_ptr<Context> _context;
};