#include <Engine/Service/WorldService.hpp>
#include <Engine/Service/Imgui/ImguiService.hpp>
#include <Engine/Service/Render/RenderService.hpp>
#include <Engine/Service/Window/WindowService.hpp>
#include <Engine/Registration.hpp>
#include <Engine/System/RenderSystem.hpp>
#include <Engine/System/TransformSystem.hpp>
#include <Engine/System/SkyboxSystem.hpp>
#include <Core/Profiling.hpp>

RTTR_REGISTRATION
{
    engine::registration::Service<engine::WorldService>("engine::WorldService")
        .UpdateBefore<engine::ImguiService>()
        .UpdateAfter<engine::WindowService>()
        .UpdateAfter<engine::RenderService>();
}

namespace engine
{

WorldService::WorldService()
{
    m_world = std::make_unique<ecs::World>("Test world");

    m_world->GetSystemManager()->Add<TransformSystem>();
    m_world->GetSystemManager()->Add<RenderSystem>();
    m_world->GetSystemManager()->Add<CameraSystem>();
    m_world->GetSystemManager()->Add<SkyboxSystem>();

    m_world->GetSystemManager()->UpdateDependenciesOrder();
}

WorldService::~WorldService()
{
}

void WorldService::Update(float dt)
{
    PROFILER_CPU_ZONE;
    m_world->Update(dt);
}

void WorldService::PostUpdate(float dt)
{
    PROFILER_CPU_ZONE;
}

} // engine