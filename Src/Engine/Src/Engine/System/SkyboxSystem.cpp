#include <Engine/System/SkyboxSystem.hpp>
#include <Engine/System/RenderSystem.hpp>
#include <Engine/System/TransformSystem.hpp>
#include <RHI/Pipeline.hpp>

RTTR_REGISTRATION
{
    using namespace engine::ecs;

    engine::registration::System<engine::SkyboxSystem>("engine::SkyboxSystem")
        .UpdateAfter<engine::RenderSystem>();

    engine::registration::Component<engine::SkyboxComponent>(Component::Type::ENGINE, "engine::SkyboxComponent");
}

namespace
{
    constexpr float skyboxVBRaw[] = {
        // back face
        -1.0f, -1.0f, -1.0f,   // bottom-left
        1.0f, 1.0f, -1.0f,   // top-right
        1.0f, -1.0f, -1.0f,   // bottom-right
        1.0f, 1.0f, -1.0f,   // top-right
        -1.0f, -1.0f, -1.0f,   // bottom-left
        -1.0f, 1.0f, -1.0f,   // top-left
        // front face
        -1.0f, -1.0f, 1.0f,   // bottom-left
        1.0f, -1.0f, 1.0f,   // bottom-right
        1.0f, 1.0f, 1.0f,   // top-right
        1.0f, 1.0f, 1.0f,   // top-right
        -1.0f, 1.0f, 1.0f,   // top-left
        -1.0f, -1.0f, 1.0f,   // bottom-left
        // left face
        -1.0f, 1.0f, 1.0f,  // top-right
        -1.0f, 1.0f, -1.0f,  // top-left
        -1.0f, -1.0f, -1.0f,  // bottom-left
        -1.0f, -1.0f, -1.0f,  // bottom-left
        -1.0f, -1.0f, 1.0f,  // bottom-right
        -1.0f, 1.0f, 1.0f,  // top-right
        // right face
        1.0f, 1.0f, 1.0f,   // top-left
        1.0f, -1.0f, -1.0f,   // bottom-right
        1.0f, 1.0f, -1.0f,   // top-right
        1.0f, -1.0f, -1.0f,  // bottom-right
        1.0f, 1.0f, 1.0f,   // top-left
        1.0f, -1.0f, 1.0f,   // bottom-left
        // bottom face
        -1.0f, -1.0f, -1.0f,   // top-right
        1.0f, -1.0f, -1.0f,   // top-left
        1.0f, -1.0f, 1.0f,   // bottom-left
        1.0f, -1.0f, 1.0f,   // bottom-left
        -1.0f, -1.0f, 1.0f,   // bottom-right
        -1.0f, -1.0f, -1.0f,   // top-right
        // top face
        -1.0f, 1.0f, -1.0f,  // top-left
        1.0f, 1.0f, 1.0f,   // bottom-right
        1.0f, 1.0f, -1.0f,   // top-right
        1.0f, 1.0f, 1.0f,   // bottom-right
        -1.0f, 1.0f, -1.0f,   // top-left
        -1.0f, 1.0f, 1.0f,    // bottom-left
    };
}

namespace engine
{

SkyboxSystem::SkyboxSystem(ecs::World* world) : System(world)
{
    rhi::BufferDescriptor desc;
    desc.m_memoryType = rhi::MemoryType::CPU_GPU;
    desc.m_name = "SkyboxVB";
    desc.m_size = sizeof(skyboxVBRaw);
    desc.m_type = rhi::BufferType::VERTEX;

    auto& rs = Instance().Service<RenderService>();
    m_skyboxVB = rs.CreateBuffer(desc, skyboxVBRaw);
}

void SkyboxSystem::Update(float dt)
{
    PROFILER_CPU_ZONE;

    CameraUB cameraUB{};

    for (const auto [e, c, t] : W()->View<CameraComponent, TransformComponent>())
    {
        if (!c.m_active)
        {
            continue;
        }

        cameraUB.m_position = glm::vec4(t.m_position, 1.0f);
        cameraUB.m_projView = c.m_proj * glm::mat4(glm::mat3(c.m_view));
        break;
    }

    auto& rs = Instance().Service<RenderService>();

    bool skyboxDrawn = false;
    for (auto [e, skybox] : W()->View<SkyboxComponent>())
    {
        ENGINE_ASSERT_WITH_MESSAGE(!skyboxDrawn, "World can't have two skybox components!");

        skybox.m_skyboxMaterial->Material()->UpdateBuffer(1, cameraUB);

        const auto pipeline = rs.Pipeline(skybox.m_skyboxMaterial);
        rs.BeginPass(pipeline);
        rs.BindMaterial(skybox.m_skyboxMaterial);

        rs.Draw(m_skyboxVB, pipeline->VertexCount(m_skyboxVB));

        rs.EndPass(pipeline);
        skyboxDrawn = true;
    }
}

} // engine