#pragma once

#include "Components.hpp"
#include <entt/entt.hpp>

namespace RightEngine
{
    class Entity;

    class Scene : public std::enable_shared_from_this<Scene>
    {
    public:
        virtual void OnUpdate(float deltaTime);

        const std::shared_ptr<Entity>& GetRootNode() const;

        entt::registry& GetRegistry();
        std::shared_ptr<Entity> CreateEntity(const std::string& name = "New entity", bool addToRoot = false);
        std::shared_ptr<Entity> CreateEntityWithGuid(const std::string& name, const xg::Guid& guid, bool addToRoot = false);
        void DestroyEntity(const std::shared_ptr<Entity>& node);

        void UpdateNodeTransformRecursively(const std::shared_ptr<Entity>& node);

        void SetName(std::string_view aName)
        { name = aName; }
        const std::string& GetName() const
        { return name; }

        static std::shared_ptr<Scene> Create(bool empty = false);

    private:
        std::shared_ptr<Entity> rootNode;
        std::string name{ "Scene" };
        entt::registry registry;
        std::mutex m_mutex;
        std::atomic_uint32_t colorId = 0;

    private:
        friend class Entity;
    };
}