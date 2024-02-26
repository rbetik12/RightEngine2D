#include <Engine/ECS/World.hpp>

namespace engine::ecs
{

World::World(std::string_view name) : m_name(name)
{
    m_entityManager = std::make_unique<EntityManager>(this);
    m_systemManager = std::make_unique<SystemManager>(this);
}

World::~World()
{
}

void World::Update(float dt)
{
    PROFILER_CPU_ZONE;
    PROFILER_CPU_ZONE_SET_NAME(m_name.c_str(), m_name.size());

    m_entityManager->Update();
    m_systemManager->Update(dt);
}

} // engine::ecs
