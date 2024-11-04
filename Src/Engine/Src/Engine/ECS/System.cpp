#include <Engine/ECS/System.hpp>

namespace engine::ecs
{

ISystem::ISystem(World* world) : m_world(world)
{
}

} // engine::ecs