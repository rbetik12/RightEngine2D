#pragma once

#include <Engine/Config.hpp>
#include <Engine/Assert.hpp>

namespace engine
{

class ENGINE_API Engine
{
public:
	Engine(int argCount, char* argPtr[]);
	~Engine();

	int Run();

	friend Engine& instance();

private:
	inline static Engine* s_instance = nullptr;
};

ENGINE_FORCE_INLINE Engine& instance()
{
	ENGINE_ASSERT(Engine::s_instance);
	return *Engine::s_instance;
}

} // namespace engine