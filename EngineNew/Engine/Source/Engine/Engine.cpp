#include <Engine/Engine.hpp>
#include <Engine/Timer.hpp>

namespace engine
{

Engine::Engine(int argCount, char* argPtr[])
{
	Timer timer;
	s_instance = this;

	core::log::info("Engine was initialized successfully for {} ms", timer.TimeInMilliseconds());
}

Engine::~Engine()
{
	core::log::info("Engine was destroyed successfully");
}

int Engine::Run()
{
	return 0;
}

} // namespace engine

