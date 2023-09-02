#include "Scene.hpp"
#include "Assert.hpp"
#include "AssetManager.hpp"
#include "Entity.hpp"

using namespace RightEngine;

std::shared_ptr<Scene> Scene::Create(bool empty)
{
    auto sceneRawPtr = new Scene();

    std::shared_ptr<Scene> scene;
    scene.reset(sceneRawPtr);
    scene->rootNode = scene->CreateEntity();

    if (!empty)
    {
        auto camera = scene->CreateEntity("Editor camera");
        auto& cc = camera->AddComponent<CameraComponent>();
        cc.isPrimary = true;
        auto skybox = scene->CreateEntity("Skybox");
        auto& sc = skybox->AddComponent<SkyboxComponent>();
        sc.environmentHandle = AssetManager::Get().GetDefaultSkybox();
        scene->rootNode->AddChild(camera);
        scene->rootNode->AddChild(skybox);
    }

    return scene;
}

void RightEngine::Scene::OnUpdate(float deltaTime)
{
    UpdateNodeTransformRecursively(rootNode);
}

const std::shared_ptr<Entity>& Scene::GetRootNode() const
{
    return rootNode;
}

std::shared_ptr<Entity> Scene::CreateEntity(const std::string& name, bool addToRoot)
{
    std::shared_ptr<Entity> entity = std::make_shared<Entity>(registry.create(), shared_from_this());
    entity->AddComponent<TransformComponent>();
    auto& tag = entity->AddComponent<TagComponent>(name);
    tag.colorId = colorId++;

    if (addToRoot)
    {
        rootNode->AddChild(entity);
    }

    return entity;
}

void Scene::DestroyEntity(const std::shared_ptr<Entity>& node)
{
    registry.destroy(node->entityHandle);
}

void Scene::UpdateNodeTransformRecursively(const std::shared_ptr<Entity>& node)
{
    auto& transform = node->GetComponent<TransformComponent>();
    const auto parentPtr = node->parent.lock();
    if (parentPtr)
    {
        transform.RecalculateTransform(parentPtr->GetComponent<TransformComponent>());
    }
    else
    {
        transform.RecalculateTransform();
    }

    for (const auto& child: node->children)
    {
        UpdateNodeTransformRecursively(child);
    }
}

entt::registry& Scene::GetRegistry()
{
    return registry;
}

std::shared_ptr<Entity> Scene::CreateEntityWithGuid(const std::string& name, const xg::Guid& guid, bool addToRoot)
{
    std::lock_guard l(m_mutex);
    auto entity = CreateEntity(name, addToRoot);
    auto& tag = entity->GetComponent<TagComponent>();
    tag.guid = guid;

    return entity;
}
