#include <Engine/Service/Resource/ResourceService.hpp>
#include <Engine/Service/Render/RenderService.hpp>
#include <Engine/Registration.hpp>

RTTR_REGISTRATION
{
    engine::registration::Service<engine::ResourceService>("engine::ResourceService")
        .UpdateBefore<engine::RenderService>();
}

namespace engine
{

ResourceService::ResourceService()
{
}

ResourceService::~ResourceService()
{
    core::log::debug("[ResourceService] Started destroying resource loaders");

    for (auto& [type, loader] : m_loadersMap)
    {
        loader.reset();
        core::log::debug("[ResourceService] Destroyed loader '{}' successfully", type.get_name());
    }

    core::log::debug("[ResourceService] Destroyed all resource loaders successfully");
}

void ResourceService::Update(float dt)
{
    PROFILER_CPU_ZONE;

    for (auto& [_, loader] : m_loadersMap)
    {
        loader->Update();
    }
}

void ResourceService::PostUpdate(float dt)
{
}

void ResourceService::InitializeLoaders()
{
    for (auto& [_, loader] : m_loadersMap)
    {
        loader->LoadSystemResources();
    }
}

} // engine