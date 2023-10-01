#include <Engine/Engine.hpp>
#include <Engine/Service/WindowService.hpp>

namespace engine
{

Engine::Engine(int argCount, char* argPtr[])
{
	m_timer.Restart();
	s_instance = this;

	m_serviceManager.RegisterService<WindowService>();

	m_running = true;

	core::log::info("Engine was initialized successfully for {} ms", m_timer.TimeInMilliseconds());
}

Engine::~Engine()
{
	m_serviceManager.Destroy();
	core::log::info("Engine was destroyed successfully");
}

int Engine::Run()
{
	core::log::info("Starting engine loop");

	m_timer.Restart();
	while (m_running)
	{
		Update();
	}

	core::log::info("Stopped engine loop");
	return 0;
}

void Engine::Stop()
{
	core::log::info("Engine loop stop requested");
	m_running = false;
}

void Engine::Update()
{
	m_timer.Stop();

	const float dt = std::max<float>(0.0001f, m_timer.TimeInMilliseconds() * 0.001f);
	m_timer.Start();

	m_serviceManager.Update(dt);
	m_serviceManager.PostUpdate(dt);
}

} // namespace engine

