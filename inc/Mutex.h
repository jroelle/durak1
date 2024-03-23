#pragma once
#include <mutex>

struct Mutex
{
	using Guard = std::lock_guard<std::mutex>;
	static std::mutex& Get();
};