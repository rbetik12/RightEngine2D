#pragma once

#include <Engine/Config.hpp>
#include <Engine/ECS/Component.hpp>
#include <Core/Type.hpp>
#include <Core/RTTRIntegration.hpp>
#include <entt/entity/registry.hpp>
#include <entt/entity/view.hpp>

#define UUID_SYSTEM_GENERATOR
#include <uuid.h>

#include <atomic>

namespace engine::ecs
{
class World;

class ENGINE_API EntityManager : public core::NonCopyable
{
public:
    struct EntityInfo
    {
        std::string m_name;
        uuids::uuid m_uuid;
    };

    EntityManager(World* world);
    ~EntityManager();

    void            Update();

    uuids::uuid        CreateEntity(std::string_view name = "");
    void            RemoveEntity(const uuids::uuid& uuid);
    void            RemoveEntity(entt::entity e);

    template<typename T, typename... Args>
    T& AddComponent(const uuids::uuid& uuid, Args&&... args)
    {
        const auto it = m_uuidToEntity.find(uuid);
        ENGINE_ASSERT(it != m_uuidToEntity.end());

        return AddComponent<T>(it->second, std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    T&                AddComponent(entt::entity e, Args&&... args)
    {
        static_assert(std::is_base_of_v<Component, T>, "Class must be a derived of engine::ecs::Component");
        ENGINE_ASSERT(rttr::type::get<T>().get_constructor().is_valid());
        ENGINE_ASSERT(!m_registry.try_get<T>(e));

        return m_registry.emplace<T>(e, std::forward<Args>(args)...);
    }

    template<typename T>
    T&                RemoveComponent(entt::entity e)
    {
        static_assert(std::is_base_of_v<Component, T>, "Class must be a derived of engine::ecs::Component");
        ENGINE_ASSERT(m_registry.try_get<T>());
        ENGINE_ASSERT(rttr::type::get<T>().get_constructor().is_valid());

        return m_registry.remove<T>(e);
    }

    template<typename T>
    T& RemoveComponent(const uuids::uuid& uuid)
    {
        const auto it = m_uuidToEntity.find(uuid);
        ENGINE_ASSERT(it != m_uuidToEntity.end());

        return RemoveComponent<T>(it->second);
    }

private:
    friend class World;

    entt::registry                                    m_registry;
    World*                                            m_world;
    std::mutex                                        m_mutex;
    eastl::vector_map<entt::entity, EntityInfo>        m_entities;
    eastl::vector_map<uuids::uuid, entt::entity>    m_uuidToEntity;
    eastl::vector<EntityInfo>                        m_pendingCreateEntities;
    eastl::vector<entt::entity>                        m_pendingDeleteEntities;
    eastl::vector<entt::entity>                        m_pendingAddComponent;
};

} // engine::ecs