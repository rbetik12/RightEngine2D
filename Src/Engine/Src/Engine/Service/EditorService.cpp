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
#include <Engine/System/SkyboxSystem.hpp>
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
    std::shared_ptr<TextureResource>        m_envTex;
    std::shared_ptr<TextureResource>        m_brickTex;
    std::shared_ptr<TextureResource>        m_whiteTex;
    std::shared_ptr<TextureResource>        m_normalTex;
    std::shared_ptr<TextureResource>        m_brdfTex;
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
    m_impl->m_monkeyMesh = std::static_pointer_cast<MeshResource>(meshLoader.Load("/System/Models/sphere.fbx"));

    while (!m_impl->m_monkeyMesh->Ready()) {}

    auto& matLoader = Instance().Service<ResourceService>().GetLoader<MaterialLoader>();
    const auto env = matLoader.LoadEnvironmentMap("/System/Textures/spree_bank_env.hdr");
    m_impl->m_envCubMap = env.m_cubemap;

    auto skyboxMaterial = resourceService.Load<MaterialResource>("/System/Materials/skybox.material");
    skyboxMaterial->Material()->SetTexture(m_impl->m_envCubMap, 3);
    skyboxMaterial->Material()->Sync();

    const auto defaultMat = rs.DefaultMaterial();

    MeshComponent meshComponent;
    meshComponent.m_mesh = m_impl->m_monkeyMesh;
    meshComponent.m_material = defaultMat;

    m_impl->m_brickTex = Instance().Service<ResourceService>().Load<TextureResource>("/System/Textures/white.png");
    m_impl->m_whiteTex = Instance().Service<ResourceService>().Load<TextureResource>("/System/Textures/white.png");
    m_impl->m_normalTex = Instance().Service<ResourceService>().Load<TextureResource>("/System/Textures/normal_map.png");
    m_impl->m_brdfTex = Instance().Service<ResourceService>().Load<TextureResource>("/System/Textures/brdf_lut.tga");

    m_impl->m_whiteTex->Wait();
    m_impl->m_normalTex->Wait();
    m_impl->m_brickTex->Wait();
    m_impl->m_brdfTex->Wait();

    defaultMat->Material()->SetTexture(m_impl->m_brickTex->Texture(), 3);
    defaultMat->Material()->SetTexture(m_impl->m_normalTex->Texture(), 4);
    defaultMat->Material()->SetTexture(m_impl->m_whiteTex->Texture(), 5);
    defaultMat->Material()->SetTexture(m_impl->m_whiteTex->Texture(), 6);
    defaultMat->Material()->SetTexture(m_impl->m_whiteTex->Texture(), 7);
    defaultMat->Material()->SetTexture(env.m_irradianceTexture, 8);
    defaultMat->Material()->SetTexture(env.m_prefilterTexture, 9);
    defaultMat->Material()->SetTexture(m_impl->m_brdfTex->Texture(), 10);
    defaultMat->Material()->Sync();

    auto& ws = Instance().Service<WorldService>();
    auto& em = ws.CurrentWorld()->GetEntityManager();

    const auto uuid = em->CreateEntity("Monkey");
    const auto cameraUuid = em->CreateEntity("Editor Camera");
    const auto dirLightUuid = em->CreateEntity("Directional Light");
    em->Update();

    em->AddComponent<MeshComponent>(uuid, meshComponent);

    CameraComponent cameraComponent{};
    cameraComponent.m_active = true;
    em->AddComponent<CameraComponent>(cameraUuid, cameraComponent);

    auto& cameraTransform = em->GetComponent<TransformComponent>(cameraUuid);
    cameraTransform.m_position = glm::vec3(0, 0, -10);

    const auto skyboxUuid = em->CreateEntity("Skybox");
    em->Update();

    auto& skybox = em->AddComponent<SkyboxComponent>(skyboxUuid);
    skybox.m_skyboxMaterial = skyboxMaterial;

    em->AddComponent<DirectionalLightComponent>(dirLightUuid);
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
