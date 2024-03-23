#include "Mutex.h"

std::mutex& Mutex::Get()
{
	static std::mutex s_mutex;
	return s_mutex;
}