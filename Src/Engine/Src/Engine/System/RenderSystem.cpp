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
        for (const auto& submesh : mesh.m_mesh->GetSubMeshList())
        {
            rs.Draw(submesh->VertexBuffer(), rs.DefaultPipeline()->VertexCount(submesh->VertexBuffer()));
        }
    }

    rs.EndPass(rs.DefaultPipeline());
}

} // engine
