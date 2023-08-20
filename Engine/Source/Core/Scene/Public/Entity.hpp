#pragma once

#include "Shader.hpp"
#include "Scene.hpp"
#include "Assert.hpp"
#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include <memory>

namespace RightEngine
{
    class Scene;

    class Entity : public std::enable_shared_from_this<Entity>
    {
    public:
        Entity() = default;
        Entity(entt::entity entityId, const std::shared_ptr<Scene>& scene);
        ~Entity() = default;

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args)
        {
            std::lock_guard l(m_mutex);
            R_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
            T& component = scene.lock()->registry.emplace<T>(entityHandle, std::forward<Args>(args)...);
            return component;
        }

        template<typename T>
        T& GetComponent()
        {
            std::lock_guard l(m_mutex);
            R_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
            return scene.lock()->registry.get<T>(entityHandle);
        }

        template<typename T>
        bool HasComponent()
        {
            std::lock_guard l(m_mutex);
            auto ptr = scene.lock()->registry.try_get<T>(entityHandle);
            return ptr ? true : false;
        }

        template<typename T>
        void RemoveComponent()
        {
            R_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
            scene.lock()->registry.remove<T>(entityHandle);
        }

        void AddChild(const std::shared_ptr<Entity>& node);
        void RemoveChild(const std::shared_ptr<Entity>& node);

        const std::vector<std::shared_ptr<Entity>>& GetChildren() const;
        std::vector<std::shared_ptr<Entity>> GetAllChildren() const;

        std::shared_ptr<Entity> GetParent() const
        { return parent.lock(); }

    protected:
        entt::entity entityHandle;

        std::weak_ptr<Entity> parent;
        std::vector<std::shared_ptr<Entity>> children;
        std::weak_ptr<Scene> scene;

    private:
        void GetAllChildren(std::vector<std::shared_ptr<Entity>>& allChildren) const;

        std::recursive_mutex m_mutex;

        friend class Scene;
    };
}