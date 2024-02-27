#include <Engine/Service/EditorService.hpp>
#include <Engine/Service/ThreadService.hpp>
#include <Engine/Service/WindowService.hpp>
#include <Engine/Service/Render/Material.hpp>
#include <Engine/Service/Render/Mesh.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Registration.hpp>
#include <Engine/Service/Imgui/ImguiService.hpp>
#include <Engine/Service/WorldService.hpp>
#include <Engine/System/RenderSystem.hpp>
#include <imgui.h>

RTTR_REGISTRATION
{
    engine::registration::Service<engine::EditorService>("engine::EditorService")
        .UpdateAfter<engine::ImguiService>()
        .PostUpdateBefore<engine::ImguiService>()
        .Domain(engine::Domain::EDITOR);
}

namespace
{

const eastl::vector<float> vertexBufferRaw =
{
    -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    0.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
};

} // unnamed

namespace engine
{

struct EditorService::Impl
{
    std::shared_ptr<render::Mesh>       m_mesh;
    std::shared_ptr<render::Material>   m_material;
    ImVec2                              m_viewportSize;
};

EditorService::EditorService()
{
    auto& ts = Instance().Service<ThreadService>();
    ts.AddBackgroundTask([]()
        {
            core::log::debug("[EditorService] Message from another thread!");
        });

    m_impl = std::make_unique<Impl>();
}

EditorService::~EditorService()
{
    m_impl.reset();
}

void EditorService::Update(float dt)
{
    PROFILER_CPU_ZONE;

    auto& rs = Instance().Service<RenderService>();
    auto& is = Instance().Service<ImguiService>();

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("General"))
        {
            bool shouldShutdownEngine = false;
            ImGui::MenuItem("Exit", nullptr, &shouldShutdownEngine);

            if (shouldShutdownEngine)
            {
                Instance().Stop();
            }

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

    ImGui::Begin("Viewport");
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();

    if (!core::math::almostEqual(m_impl->m_viewportSize.x, viewportSize.x) || !core::math::almostEqual(m_impl->m_viewportSize.y, viewportSize.y))
    {
        m_impl->m_viewportSize = { viewportSize.x, viewportSize.y };
        rs.OnResize(static_cast<uint32_t>(m_impl->m_viewportSize.x), static_cast<uint32_t>(m_impl->m_viewportSize.y));
    }

    is.Image(rs.BasicPass()->Descriptor().m_colorAttachments[0].m_texture, m_impl->m_viewportSize);

    ImGui::End();

    ImGui::Begin("Test", nullptr);
    ImGui::End();
}

void EditorService::PostUpdate(float dt)
{
    PROFILER_CPU_ZONE;
}

void EditorService::Initialize()
{
    auto& rs = Instance().Service<RenderService>();

    rhi::BufferDescriptor bufferDesc{};
    bufferDesc.m_memoryType = rhi::MemoryType::CPU_GPU;
    bufferDesc.m_type = rhi::BufferType::VERTEX;
    bufferDesc.m_size = sizeof(vertexBufferRaw[0]) * static_cast<uint32_t>(vertexBufferRaw.size());

    const auto buffer = rs.CreateBuffer(bufferDesc, vertexBufferRaw.data());

    m_impl->m_mesh = std::make_shared<render::Mesh>(buffer);
    m_impl->m_material = std::make_shared<render::Material>(rs.DefaultShader());

    MeshComponent meshComponent;
    meshComponent.m_mesh = m_impl->m_mesh;
    meshComponent.m_material = m_impl->m_material;

    auto& ws = Instance().Service<WorldService>();
    auto& em = ws.CurrentWorld()->GetEntityManager();

    const auto uuid = em->CreateEntity("Triangle");
    em->Update();

    em->AddComponent<MeshComponent>(uuid, meshComponent);
}

} // engine
