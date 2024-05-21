#pragma once

#include <Engine/Config.hpp>
#include <Engine/ECS/SystemManager.hpp>
#include <Engine/ECS/EntityManager.hpp>

namespace engine::ecs
{

class ENGINE_API World
{
public:
    World(std::string_view name);
    ~World();

    void    Update(float dt);

    auto    View()
    {
        return m_entityManager->m_registry.view<entt::entity>();
    }

    template<typename T, typename... Types>
    auto    View()
    {
        return m_entityManager->m_registry.view<T, Types...>().each();
    }

    std::unique_ptr<SystemManager>& GetSystemManager() { return m_systemManager; }
    std::unique_ptr<EntityManager>& GetEntityManager() { return m_entityManager; }

private:
    std::unique_ptr<SystemManager>  m_systemManager;
    std::unique_ptr<EntityManager>  m_entityManager;
    std::string                     m_name;
};

} // engine::ecs