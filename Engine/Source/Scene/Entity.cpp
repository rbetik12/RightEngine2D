#include "Entity.hpp"
#include "Renderer.hpp"
#include <glm/ext/matrix_transform.hpp>

using namespace RightEngine;

Entity::Entity(entt::entity entityHandle, const std::shared_ptr<Scene>& scene)
{
    this->entityHandle = entityHandle;
    this->scene = scene;
}

void Entity::AddChild(const std::shared_ptr<Entity>& node)
{
    children.push_back(node);
    node->parent = shared_from_this();
}

void Entity::RemoveChild(const std::shared_ptr<Entity>& node)
{
    for (int index = 0; index < children.size(); index++)
    {
        if (node == children[index])
        {
            children.erase(children.begin() + index);
            break;
        }
    }
}

const std::vector<std::shared_ptr<Entity>>& Entity::GetChildren() const
{
    return children;
}

std::vector<std::shared_ptr<Entity>> Entity::GetAllChildren() const
{
    std::vector<std::shared_ptr<Entity>> allChildren;
    GetAllChildren(allChildren);
    return allChildren;
}

void Entity::GetAllChildren(std::vector<std::shared_ptr<Entity>>& allChildren) const
{
    for (const auto& node: children)
    {
        node->GetAllChildren(allChildren);
        allChildren.push_back(node);
    }
}
