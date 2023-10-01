#pragma once

#include <Engine/Config.hpp>
#include <Engine/Assert.hpp>
#include <Engine/Timer.hpp>
#include <Engine/Service/ServiceManager.hpp>
#include <Core/Type.hpp>

namespace engine
{

class ENGINE_API Engine final : private core::NonCopyable
{
public:
	Engine(int argCount, char* argPtr[]);
	~Engine();

	int Run();
	void Stop();

	friend Engine& Instance();

private:
	void Update();

private:
	inline static Engine* s_instance = nullptr;

	bool			m_running = false;
	Timer			m_timer;
	ServiceManager	m_serviceManager;
};

ENGINE_FORCE_INLINE Engine& Instance()
{
	ENGINE_ASSERT(Engine::s_instance);
	return *Engine::s_instance;
}

} // namespace engine