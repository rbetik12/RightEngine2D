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
    std::shared_ptr<render::Material>       m_equrectangleMaterial;
    std::shared_ptr<rhi::Texture>           m_envCubMap;
    std::shared_ptr<rhi::Pipeline>          m_computePipeline;
    std::shared_ptr<TextureResource>        m_envTex;
    std::shared_ptr<MeshResource>           m_monkeyMesh;
    eastl::vector<std::shared_ptr<Panel>>   m_panels;
    std::shared_ptr<ViewportPanel>          m_viewportPanel;
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
    m_impl->m_viewportPanel = std::static_pointer_cast<ViewportPanel>(m_impl->m_panels.back());
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

    auto& rs = Instance().Service<RenderService>();
    
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

    rs.BeginComputePass(m_impl->m_computePipeline);
    rs.BindMaterial(m_impl->m_equrectangleMaterial, m_impl->m_computePipeline);
    rs.Dispatch(m_impl->m_envCubMap->Width() / 32, m_impl->m_envCubMap->Height() / 32, 6);
    rs.EndComputePass(m_impl->m_computePipeline);
}

void EditorService::PostUpdate(float dt)
{
    PROFILER_CPU_ZONE;
}

void EditorService::Initialize()
{
    auto& rs = Instance().Service<RenderService>();
    // auto& vfs = Instance().Service<io::VirtualFilesystemService>();
    auto& resourceService = Instance().Service<ResourceService>();

    auto& meshLoader = resourceService.GetLoader<MeshLoader>();
    m_impl->m_monkeyMesh = std::static_pointer_cast<MeshResource>(meshLoader.Load("/Meshes/monkey.fbx"));

    while (!m_impl->m_monkeyMesh->Ready()) {}
    m_impl->m_material = std::make_shared<render::Material>(rs.DefaultShader());

    MeshComponent meshComponent;
    meshComponent.m_mesh = m_impl->m_monkeyMesh;
    meshComponent.m_material = m_impl->m_material;

    auto& ws = Instance().Service<WorldService>();
    auto& em = ws.CurrentWorld()->GetEntityManager();

    const auto uuid = em->CreateEntity("Monkey");
    const auto cameraUuid = em->CreateEntity("Editor Camera");
    em->Update();

    em->AddComponent<MeshComponent>(uuid, meshComponent);

    CameraComponent cameraComponent{};
    cameraComponent.m_active = true;
    em->AddComponent<CameraComponent>(cameraUuid, cameraComponent);

    // const auto computeShaderPath = "/System/Shaders/equirectangle_to_cubemap.glsl";
    // const auto computeCompiledShader = rs.ShaderCompiler()->Compile(vfs.Absolute(io::fs::path(computeShaderPath)).generic_u8string(), rhi::ShaderType::COMPUTE);
    //
    // rhi::ShaderDescriptor computeShaderDesc{};
    // computeShaderDesc.m_path = computeShaderPath;
    // computeShaderDesc.m_blobByStage = computeCompiledShader.m_stageBlob;
    // computeShaderDesc.m_name = "Compute";
    // computeShaderDesc.m_type = rhi::ShaderType::COMPUTE;
    // computeShaderDesc.m_reflection = computeCompiledShader.m_reflection;
    //
    // const auto computeShader = rs.CreateShader(computeShaderDesc);
    //
    // auto& texLoader = resourceService.GetLoader<TextureLoader>();
    //
    // m_impl->m_envTex = std::static_pointer_cast<TextureResource>(texLoader.Load("/System/Textures/spree_bank_env.hdr"));
    //
    // rhi::TextureDescriptor envCubemapDesc{};
    // envCubemapDesc.m_type = rhi::TextureType::TEXTURE_CUBEMAP;
    // envCubemapDesc.m_format = rhi::Format::RGBA16_SFLOAT;
    // envCubemapDesc.m_layersAmount = 6;
    // envCubemapDesc.m_mipLevels = 1;
    // envCubemapDesc.m_width = 1024;
    // envCubemapDesc.m_height = 1024;
    //
    // m_impl->m_envCubMap = rs.CreateTexture(envCubemapDesc);
    // m_impl->m_equrectangleMaterial = std::make_shared<render::Material>(computeShader);
    //
    // while (!m_impl->m_envTex->Ready())
    // {}
    //
    // const auto computePass = std::make_shared<rhi::ComputePass>();
    // computePass->m_textures.emplace_back(m_impl->m_envTex->Texture());
    // computePass->m_storageTextures.emplace_back(m_impl->m_envCubMap);
    //
    // rhi::PipelineDescriptor computePipelineDesc{};
    // computePipelineDesc.m_compute = true;
    // computePipelineDesc.m_computePass = computePass;
    // computePipelineDesc.m_shader = computeShader;
    //
    // m_impl->m_computePipeline = rs.CreatePipeline(computePipelineDesc);
    //
    // m_impl->m_equrectangleMaterial->SetTexture(m_impl->m_envCubMap, 0);
    // m_impl->m_equrectangleMaterial->SetTexture(m_impl->m_envTex->Texture(), 1);
    // m_impl->m_equrectangleMaterial->Sync();
}

glm::ivec2 EditorService::ViewportSize() const
{
    return m_impl->m_viewportPanel->Size();
}

} // engine
