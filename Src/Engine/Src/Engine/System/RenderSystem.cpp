#include <Engine/System/RenderSystem.hpp>
#include <Engine/Registration.hpp>

#include "RHI/Pipeline.hpp"

RTTR_REGISTRATION
{
    using namespace engine::ecs;

    engine::registration::System<engine::RenderSystem>("engine::RenderSystem");

    engine::registration::Component<engine::MeshComponent>(Component::Type::ENGINE, "engine::MeshComponent");
}

namespace engine
{

RenderSystem::RenderSystem(ecs::World* world) : System(world)
{
}

void RenderSystem::Update(float dt)
{
    PROFILER_CPU_ZONE;
    auto& rs = Instance().Service<RenderService>();

    rs.BeginPass(rs.DefaultPipeline());

    for (auto [e, mesh] : W()->View<MeshComponent>().each())
    {
        auto& vb = mesh.m_mesh->VertexBuffer();
        rs.Draw(vb, rs.DefaultPipeline()->VertexCount(vb));
    }

    rs.EndPass(rs.DefaultPipeline());
}

} // engine
