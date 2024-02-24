#include <Engine/ECS/World.hpp>

namespace engine::ecs
{

World::World()
{
	m_entityManager = std::make_unique<EntityManager>(this);
	m_systemManager = std::make_unique<SystemManager>(this);
}

World::~World()
{
}

void World::Update(float dt)
{
	m_entityManager->Update();
	m_systemManager->Update(dt);
}

} // engine::ecs
