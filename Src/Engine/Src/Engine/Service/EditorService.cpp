#include <Engine/Service/EditorService.hpp>
#include <Engine/Service/ThreadService.hpp>
#include <Engine/Service/Window/WindowService.hpp>
#include <Engine/Service/Render/Material.hpp>
#include <Engine/Service/Render/Mesh.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Registration.hpp>
#include <Engine/Service/Imgui/ImguiService.hpp>
#include <Engine/Service/WorldService.hpp>
#include <Engine/Service/Filesystem/VirtualFilesystemService.hpp>
#include <Engine/Service/Resource/ResourceService.hpp>
#include <Engine/Service/Resource/TextureResource.hpp>
#include <Engine/Service/Resource/MeshResource.hpp>
#include <Engine/System/RenderSystem.hpp>
#include <Engine/System/TransformSystem.hpp>
#include <Engine/Editor/Panel.hpp>
#include <Engine/Editor/ViewportPanel.hpp>
#include <Engine/Editor/EntityTreePanel.hpp>
#include <Engine/Editor/ComponentPanel.hpp>
#include <RHI/Pipeline.hpp>
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

using namespace editor;

struct EditorService::Impl
{
    std::shared_ptr<render::Mesh>           m_mesh;
    std::shared_ptr<render::Material>       m_material;
    std::shared_ptr<rhi::Texture>           m_envCubMap;
    std::shared_ptr<rhi::Pipeline>          m_computePipeline;
    std::shared_ptr<TextureResource>        m_envTex;
    std::shared_ptr<MeshResource>           m_monkeyMesh;
    eastl::vector<std::shared_ptr<Panel>>   m_panels;
    std::shared_ptr<ViewportPanel>          m_viewportPanel;

    ImVec2                                  m_viewportSize = ImVec2(1, 1);
    entt::entity                            m_selectedEntity = C_INVALID_ENTITY;
};

EditorService::EditorService()
{
    auto& ts = Instance().Service<ThreadService>();
    ts.AddBackgroundTask([]()
        {
            core::log::debug("[EditorService] Message from another thread!");
        });

    m_impl = std::make_unique<Impl>();

    m_impl->m_panels.emplace_back(std::make_shared<ViewportPanel>());
    m_impl->m_panels.emplace_back(std::make_shared<EntityTreePanel>());
    m_impl->m_panels.emplace_back(std::make_shared<ComponentPanel>());

    m_impl->m_viewportPanel = std::static_pointer_cast<ViewportPanel>(m_impl->m_panels[0]);
}

EditorService::~EditorService()
{
    auto& rs = Instance().Service<RenderService>();
    rs.WaitAll();

    m_impl.reset();
}

void EditorService::Update(float dt)
{
    PROFILER_CPU_ZONE;
    
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

    for (auto& panel : m_impl->m_panels)
    {
        panel->Draw();
    }
}

void EditorService::PostUpdate(float dt)
{
    PROFILER_CPU_ZONE;
}

void EditorService::SelectedEntity(entt::entity e)
{
    m_impl->m_selectedEntity = e;
}

entt::entity EditorService::SelectedEntity()
{
    return m_impl->m_selectedEntity;
}

void EditorService::Initialize()
{
    auto& rs = Instance().Service<RenderService>();
    auto& resourceService = Instance().Service<ResourceService>();

    auto& meshLoader = resourceService.GetLoader<MeshLoader>();
    m_impl->m_monkeyMesh = std::static_pointer_cast<MeshResource>(meshLoader.Load("/Meshes/monkey.fbx"));

    while (!m_impl->m_monkeyMesh->Ready()) {}

    MeshComponent meshComponent;
    meshComponent.m_mesh = m_impl->m_monkeyMesh;
    meshComponent.m_material = rs.DefaultMaterial();

    auto& ws = Instance().Service<WorldService>();
    auto& em = ws.CurrentWorld()->GetEntityManager();

    const auto uuid = em->CreateEntity("Monkey");
    const auto cameraUuid = em->CreateEntity("Editor Camera");
    em->Update();

    em->AddComponent<MeshComponent>(uuid, meshComponent);

    CameraComponent cameraComponent{};
    cameraComponent.m_active = true;
    em->AddComponent<CameraComponent>(cameraUuid, cameraComponent);

    auto& cameraTransform = em->GetComponent<TransformComponent>(cameraUuid);
    cameraTransform.m_position = glm::vec3(0, 0, -90);
}

glm::ivec2 EditorService::ViewportSize() const
{
    return m_impl->m_viewportPanel->Size();
}

bool EditorService::IsViewportHovered() const
{
    return m_impl->m_viewportPanel->IsHovered();
}

} // engine
