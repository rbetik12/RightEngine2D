#include <Engine/System/TransformSystem.hpp>
#include <Engine/Registration.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

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

    for (auto [e, t] : W()->View<TransformComponent>())
    {
        const glm::mat4 rotationMatrix = glm::toMat4(glm::quat(t.m_rotation));

        t.m_worldTransform = glm::translate(glm::mat4(1.0f), t.m_position) * rotationMatrix * glm::scale(glm::mat4(1.0f), t.m_scale);
    }
}

} // engine
