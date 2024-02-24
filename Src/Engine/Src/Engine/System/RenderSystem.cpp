#include <Engine/System/RenderSystem.hpp>
#include <Engine/Registration.hpp>

RTTR_REGISTRATION
{
	engine::registration::System<engine::RenderSystem>("engine::RenderSystem");
}

namespace engine
{

RenderSystem::RenderSystem(ecs::World* world) : System(world)
{
}

void RenderSystem::Update(float dt)
{
	PROFILER_CPU_ZONE;
}

} // engine
