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

    void                Update();

    uuids::uuid         CreateEntity(std::string_view name = "");
    void                RemoveEntity(const uuids::uuid& uuid);
    void                RemoveEntity(entt::entity e);

    const EntityInfo&   GetEntityInfo(entt::entity e);

    template<typename T>
    T*                  TryGetComponent(entt::entity e)
    {
        static_assert(std::is_base_of_v<Component, T>, "Class must be a derived of engine::ecs::Component");
        ENGINE_ASSERT(engine::registration::helpers::typeRegistered<T>());
        return m_registry.try_get<T>(e);
    }

    template<typename T>
    T*                  TryGetComponent(const uuids::uuid& uuid)
    {
        const auto it = m_uuidToEntity.find(uuid);
        ENGINE_ASSERT(it != m_uuidToEntity.end());

        return TryGetComponent<T>(it->second);
    }

    template<typename T>
    T&                  GetComponent(entt::entity e)
    {
        ENGINE_ASSERT(engine::registration::helpers::typeRegistered<T>());
        ENGINE_ASSERT(TryGetComponent<T>(e));

        return m_registry.get<T>(e);
    }

    template<typename T>
    T&                  GetComponent(const uuids::uuid& uuid)
    {
        const auto it = m_uuidToEntity.find(uuid);
        ENGINE_ASSERT(it != m_uuidToEntity.end());

        return GetComponent<T>(it->second);
    }

    template<typename T, typename... Args>
    T&                  AddComponent(const uuids::uuid& uuid, Args&&... args)
    {
        const auto it = m_uuidToEntity.find(uuid);
        ENGINE_ASSERT(it != m_uuidToEntity.end());

        return AddComponent<T>(it->second, std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    T&                  AddComponent(entt::entity e, Args&&... args)
    {
        static_assert(std::is_base_of_v<Component, T>, "Class must be a derived of engine::ecs::Component");
        ENGINE_ASSERT(engine::registration::helpers::typeRegistered<T>());
        ENGINE_ASSERT(!TryGetComponent<T>(e));

        return m_registry.emplace<T>(e, std::forward<Args>(args)...);
    }

    template<typename T>
    bool                 RemoveComponent(entt::entity e)
    {
        static_assert(std::is_base_of_v<Component, T>, "Class must be a derived of engine::ecs::Component");
        ENGINE_ASSERT(TryGetComponent<T>(e));
        ENGINE_ASSERT(engine::registration::helpers::typeRegistered<T>());

        return m_registry.remove<T>(e) != 0;
    }

    template<typename T>
    bool                 RemoveComponent(const uuids::uuid& uuid)
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
    eastl::vector_map<entt::entity, EntityInfo>       m_entities;
    eastl::vector_map<uuids::uuid, entt::entity>      m_uuidToEntity;
    eastl::vector<EntityInfo>                         m_pendingCreateEntities;
    eastl::vector<entt::entity>                       m_pendingDeleteEntities;
    eastl::vector<entt::entity>                       m_pendingAddComponent;
};

} // engine::ecs