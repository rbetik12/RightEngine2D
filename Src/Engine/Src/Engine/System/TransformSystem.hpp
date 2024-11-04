#pragma once

#include <Engine/Config.hpp>
#include <Engine/ECS/System.hpp>
#include <Engine/ECS/World.hpp>
#include <Engine/ECS/Component.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/glm.hpp>

namespace engine
{

struct ENGINE_API TransformComponent : public ecs::Component
{
    glm::quat m_rotation = glm::identity<glm::quat>();
    glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 m_scale = glm::vec3(1.0f, 1.0f, 1.0f);

    glm::mat4 m_worldTransform = glm::mat4(1.0f);
};

class ENGINE_API TransformSystem : public ecs::System<TransformSystem>
{
public:
    TransformSystem(ecs::World* world);
    virtual ~TransformSystem() = default;

    virtual void Update(float dt) override;
};

} // engine