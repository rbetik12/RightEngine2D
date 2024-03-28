#include <Engine/System/TransformSystem.hpp>
#include <Engine/Registration.hpp>

RTTR_REGISTRATION
{
    using namespace engine::ecs;

    engine::registration::System<engine::TransformSystem>("engine::TransformSystem");

    engine::registration::Component<engine::TransformComponent>(Component::Type::ENGINE, "engine::TransformComponent");
}

namespace engine
{

TransformSystem::TransformSystem(ecs::World* world) : System(world)
{
}

void TransformSystem::Update(float dt)
{
    PROFILER_CPU_ZONE;
}

} // engine
