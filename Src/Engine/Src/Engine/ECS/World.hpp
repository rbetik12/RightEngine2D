#pragma once

#include <Engine/Config.hpp>
#include <Engine/ECS/SystemManager.hpp>
#include <Engine/ECS/EntityManager.hpp>

namespace engine::ecs
{

class ENGINE_API World
{
public:
	World();
	~World();

	void	Update(float dt);

	template<typename T, typename... Types>
	auto	View()
	{
		return m_entityManager->m_registry.view<T, Types...>();
	}

	std::unique_ptr<SystemManager>& GetSystemManager() { return m_systemManager; }
	std::unique_ptr<EntityManager>& GetEntityManager() { return m_entityManager; }

private:
	std::unique_ptr<SystemManager> m_systemManager;
	std::unique_ptr<EntityManager> m_entityManager;
};

} // engine::ecs