#include <Engine/ECS/System.hpp>

namespace engine::ecs
{

System::System(World* world) : m_world(world)
{
}

} // engine::ecs