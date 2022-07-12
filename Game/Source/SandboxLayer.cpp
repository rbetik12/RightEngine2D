#include "SandboxLayer.hpp"
#include "Renderer.hpp"
#include "EditorCamera.hpp"
#include "RendererCommand.hpp"
#include "UniformBuffer.hpp"
#include "Entity.hpp"
#include "Components.hpp"
#include "Texture.hpp"
#include "TextureLoader.hpp"
#include "Panels/PropertyPanel.hpp"
#include "EnvironmentMapLoader.hpp"
#include "Utils/MeshLoader.hpp"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <imgui.h>

using namespace RightEngine;

namespace
{
    float skyboxVertices[] = {
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

    struct ShaderLight
    {
        glm::vec4 color;
        glm::vec4 position;
        glm::vec4 rotation;
        float intensity;
        int type;
        glm::vec2 _padding_;
    };

    struct LightBuffer
    {
        glm::ivec4 lightsAmount;
        ShaderLight light[30];
    };

    enum class TextureSlot
    {
        ALBEDO_TEXTURE_SLOT = 0,
        NORMAL_TEXTURE_SLOT,
        METALLIC_TEXTURE_SLOT,
        ROUGHNESS_TEXTURE_SLOT,
        AO_TEXTURE_SLOT,
        SKYBOX_TEXTURE_SLOT,
        PREFILTER_TEXTURE_SLOT,
        IRRADIANCE_TEXTURE_SLOT,
        BRDF_LUT_TEXTURE_SLOT
    };

    enum class GeometryType
    {
        NONE,
        CUBE,
        PLANE
    };

    const uint32_t width = 1280;
    const uint32_t height = 720;

    struct LayerSceneData
    {
        std::shared_ptr<UniformBuffer> materialUniformBuffer;
        std::shared_ptr<EditorCamera> camera;
        std::shared_ptr<Entity> skyboxCube;
        std::shared_ptr<Shader> skyboxShader;
        ImVec2 viewportSize{ width, height };
        uint32_t newEntityId{ 1 };
        uint32_t selectedNodeId{ 0 };
        PropertyPanel propertyPanel;
        MeshLoader meshLoader;
        std::shared_ptr<MeshNode> gun;
        TextureLoader textureLoader;
        std::shared_ptr<UniformBuffer> lightUniformBuffer;
    };

    static LayerSceneData sceneData;


    void TryTextureBind(MaterialData& materialData, const std::shared_ptr<Texture>& texture, TextureSlot slot)
    {
        bool hasTexture = false;
        if (texture)
        {
            texture->Bind(static_cast<uint32_t>(slot));
            hasTexture = true;
        }

        switch (slot)
        {
            case TextureSlot::ALBEDO_TEXTURE_SLOT:
                materialData.hasAlbedo = hasTexture;
                break;
            case TextureSlot::NORMAL_TEXTURE_SLOT:
                materialData.hasNormal = hasTexture;
                break;
            case TextureSlot::METALLIC_TEXTURE_SLOT:
                materialData.hasMetallic = hasTexture;
                break;
            case TextureSlot::ROUGHNESS_TEXTURE_SLOT:
                materialData.hasRoughness = hasTexture;
                break;
            case TextureSlot::AO_TEXTURE_SLOT:
                materialData.hasAO = hasTexture;
                break;
            default:
            R_ASSERT(false, "Unknown texture slot!");
        }
    }

    void BindTextures(MaterialData& material, const TextureData& textures)
    {
        TryTextureBind(material, textures.albedo, TextureSlot::ALBEDO_TEXTURE_SLOT);
        TryTextureBind(material, textures.normal, TextureSlot::NORMAL_TEXTURE_SLOT);
        TryTextureBind(material, textures.metallic, TextureSlot::METALLIC_TEXTURE_SLOT);
        TryTextureBind(material, textures.roughness, TextureSlot::ROUGHNESS_TEXTURE_SLOT);
        TryTextureBind(material, textures.ao, TextureSlot::AO_TEXTURE_SLOT);
    }

    void AddTag(std::shared_ptr<Entity>& entity, const std::string& name = "")
    {
        Tag tag;
        tag.id = sceneData.newEntityId++;
        if (name.empty())
        {
            tag.name = "Entity" + std::to_string(tag.id);
        }
        else
        {
            tag.name = name;
        }
        entity->AddComponent<Tag>(tag);
    }

    void CreateEntitiesFromMeshTree(std::shared_ptr<Entity>& entity,
                                    const std::shared_ptr<MeshNode>& tree,
                                    const std::shared_ptr<Scene>& scene)
    {
        if (!entity)
        {
            entity = scene->CreateEntity();
            AddTag(entity);
        }

        for (auto& mesh: tree->meshes)
        {
            auto newMeshEntity = scene->CreateEntity();
            AddTag(newMeshEntity);
            newMeshEntity->AddComponent<Mesh>(*mesh);
            entity->AddChild(newMeshEntity);
        }

        for (auto& child: tree->children)
        {
            if (child->meshes.empty())
            {
                continue;
            }
            auto newChild = scene->CreateEntity();
            AddTag(newChild);
            entity->AddChild(newChild);
            CreateEntitiesFromMeshTree(newChild, child, scene);
        }
    }

    std::shared_ptr<Entity> CreateTestSceneNode(const std::shared_ptr<Scene>& scene,
                                                const std::shared_ptr<MeshNode>& meshTree = nullptr,
                                                GeometryType type = GeometryType::NONE)
    {
        std::shared_ptr<Entity> node = scene->CreateEntity();
        if (meshTree)
        {
            CreateEntitiesFromMeshTree(node, meshTree, scene);
        }
        else
        {
            if (type != GeometryType::NONE)
            {
                std::shared_ptr<Mesh> mesh = nullptr;
                switch (type)
                {
                    case GeometryType::CUBE:
                        mesh = MeshBuilder::CubeGeometry();
                        break;
                    case GeometryType::PLANE:
                        mesh = MeshBuilder::PlaneGeometry();
                        break;
                }
                node->AddComponent<Mesh>(std::move(*mesh));
            }
        }
        AddTag(node);
        return node;
    }

    void ImGuiAddTreeNodeChildren(const std::shared_ptr<Entity>& node)
    {
        for (const auto& entity: node->GetChildren())
        {
            const auto& tag = entity->GetComponent<Tag>();
            ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow;
            bool node_open = ImGui::TreeNodeEx((void*) tag.id, node_flags, "%s", tag.name.c_str());
            if (ImGui::IsItemClicked())
            {
                sceneData.propertyPanel.SetSelectedEntity(entity);
            }
            if (node_open)
            {
                ImGuiAddTreeNodeChildren(entity);
                ImGui::TreePop();
            }
        }
    }
}

void SandboxLayer::OnAttach()
{
    sceneData.gun = sceneData.meshLoader.Load("/Assets/Models/Gun/cerberus_2.fbx");

    sceneData.camera = std::make_shared<EditorCamera>(glm::vec3(0, 10, -15),
                                                      glm::vec3(0, 1, 0));
    scene = Scene::Create();

    std::shared_ptr<Entity> gun = CreateTestSceneNode(scene, sceneData.gun);
    gun->GetComponent<Tag>().name = "Gun";

    auto& gunMesh = gun->GetChildren().back();
    auto& gunTransform = gunMesh->GetComponent<Transform>();
    gunTransform.SetScale({ 0.3f, 0.3f, 0.3f });
    gunTransform.SetRotation({ -90.0f, 0.0f, 0.0f });
    auto& textureData = gunMesh->GetComponent<Mesh>().GetMaterial()->textureData;
    textureData.albedo = sceneData.textureLoader.CreateTexture("/Assets/Textures/cerberus_Textures/Cerberus_A.tga");
    textureData.normal = sceneData.textureLoader.CreateTexture("/Assets/Textures/cerberus_Textures/Cerberus_N.tga");
    // TODO: Investigate why do we need mipmaps for normal texture
    textureData.normal->GenerateMipmaps();
    textureData.roughness = sceneData.textureLoader.CreateTexture("/Assets/Textures/cerberus_Textures/Cerberus_R.tga");
    textureData.metallic = sceneData.textureLoader.CreateTexture("/Assets/Textures/cerberus_Textures/Cerberus_M.tga");

    scene->SetCamera(sceneData.camera);
    scene->GetRootNode()->AddChild(gun);
    shader = Shader::Create("/Assets/Shaders/Basic/pbr.vert",
                            "/Assets/Shaders/Basic/pbr.frag");
    renderer = std::make_shared<Renderer>();

    FramebufferSpecification fbSpec;
    fbSpec.width = sceneData.viewportSize.x;
    fbSpec.height = sceneData.viewportSize.y;
    fbSpec.attachments = FramebufferAttachmentSpecification(
            {
                    FramebufferTextureSpecification(FramebufferTextureFormat::RGBA8),
                    FramebufferTextureSpecification(FramebufferTextureFormat::RGBA8),
                    FramebufferTextureSpecification(FramebufferTextureFormat::Depth),
            }
    );

    frameBuffer = std::make_shared<Framebuffer>(fbSpec);

    sceneData.materialUniformBuffer = UniformBuffer::Create(sizeof(MaterialData), 0);
    sceneData.lightUniformBuffer = UniformBuffer::Create(sizeof(LightBuffer), 1);

    sceneData.skyboxShader = Shader::Create("/Assets/Shaders/Basic/skybox.vert",
                                            "/Assets/Shaders/Basic/skybox.frag");

    EnvironmentMapLoader mapLoader;
    mapLoader.Load("/Assets/Textures/env_malibu.hdr", true);
    const auto envContext = mapLoader.GetEnvironmentContext();

    sceneData.skyboxCube = scene->CreateEntity();
    VertexBufferLayout layout;
    layout.Push<float>(3);
    Mesh mesh;
    auto vertexArray = std::make_shared<VertexArray>();
    vertexArray->AddBuffer(std::make_shared<VertexBuffer>(skyboxVertices, sizeof(skyboxVertices)), layout);
    mesh.SetVertexArray(vertexArray);
    mesh.SetVisibility(false);
    sceneData.skyboxCube->AddComponent<Mesh>(mesh);
    sceneData.skyboxCube->AddComponent<Tag>(Tag("Skybox", sceneData.newEntityId++));
    auto& skyboxComponent = sceneData.skyboxCube->AddComponent<Skybox>();
    skyboxComponent.environment = envContext;
    scene->GetRootNode()->AddChild(sceneData.skyboxCube);

    sceneData.propertyPanel.SetScene(scene);

    auto light = CreateTestSceneNode(scene, nullptr, GeometryType::NONE);
    light->GetComponent<Tag>().name = "Light";
    auto lightComponent = Light();
    lightComponent.intensity = 1000.0f;
    lightComponent.color = glm::vec3(1.0f, 0.0f, 0.0f);
    light->AddComponent<Light>(lightComponent);
    light->GetComponent<Transform>().SetPosition(glm::vec3(0, 10, -15));

    auto light1 = CreateTestSceneNode(scene, nullptr, GeometryType::NONE);
    light1->GetComponent<Tag>().name = "Light1";
    auto lightComponent1 = Light();
    lightComponent1.intensity = 1000.0f;
    lightComponent1.color = glm::vec3(1.0f, 0.0f, 0.0f);
    light1->AddComponent<Light>(lightComponent);
    light1->GetComponent<Transform>().SetPosition(glm::vec3(0, 10, 15));

    scene->GetRootNode()->AddChild(light);
    scene->GetRootNode()->AddChild(light1);
}

void SandboxLayer::OnUpdate(float ts)
{
    scene->OnUpdate();

    frameBuffer->Bind();
    RendererCommand::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderer->Configure();
    renderer->BeginScene(scene);

    LightBuffer lightBuffer;
    std::memset(&lightBuffer, 0, sizeof(LightBuffer));
    for (const auto& entityID: scene->GetRegistry().view<Light>())
    {
        const auto& transform = scene->GetRegistry().get<Transform>(entityID);
        const auto& light = scene->GetRegistry().get<Light>(entityID);
        ShaderLight shaderLight;
        shaderLight.type = static_cast<int>(light.type);
        shaderLight.position = glm::vec4(transform.GetWorldPosition(), 1);
        shaderLight.color = glm::vec4(light.color, 1);
        shaderLight.intensity = light.intensity;
        lightBuffer.light[lightBuffer.lightsAmount.x] = shaderLight;
        lightBuffer.lightsAmount.x += 1;
        switch (light.type)
        {
            case LightType::DIRECTIONAL:
                lightBuffer.lightsAmount.y += 1;
                break;
            default:
            R_ASSERT(false, "");
        }
    }

    R_ASSERT(lightBuffer.lightsAmount.x < 30, "");
    sceneData.lightUniformBuffer->SetData(&lightBuffer, sizeof(LightBuffer));

    const auto skyboxView = scene->GetRegistry().view<Skybox>();
    // TODO: Add black skybox for fallback
    R_ASSERT(!skyboxView.empty(), "No skybox was set!");
    R_ASSERT(skyboxView.size() == 1, "There must only 1 skybox in scene!");
    const auto& skyboxEntityID = skyboxView.front();
    const auto& skybox = scene->GetRegistry().get<Skybox>(skyboxEntityID);

    for (const auto& entity: scene->GetRegistry().view<Mesh>())
    {
        const auto& transform = scene->GetRegistry().get<Transform>(entity);
        const auto& mesh = scene->GetRegistry().get<Mesh>(entity);
        if (!mesh.IsVisible())
        {
            continue;
        }

        auto& materialData = mesh.GetMaterial()->materialData;
        const auto& textureData = mesh.GetMaterial()->textureData;
        BindTextures(materialData, textureData);

        shader->Bind();
        shader->SetUniform1iv("u_Textures", { 0, 1, 2, 3, 4 });
        shader->SetUniform3f("u_CameraPosition", sceneData.camera->GetPosition());
        skybox.environment.irradianceMap->Bind(static_cast<uint32_t>(TextureSlot::IRRADIANCE_TEXTURE_SLOT));
        shader->SetUniform1i("u_IrradianceMap", static_cast<uint32_t>(TextureSlot::IRRADIANCE_TEXTURE_SLOT));
        skybox.environment.prefilterMap->GetSampler()->Bind(static_cast<uint32_t>(TextureSlot::PREFILTER_TEXTURE_SLOT));
        skybox.environment.prefilterMap->Bind(static_cast<uint32_t>(TextureSlot::PREFILTER_TEXTURE_SLOT));
        shader->SetUniform1i("u_PrefilterMap", static_cast<uint32_t>(TextureSlot::PREFILTER_TEXTURE_SLOT));
        skybox.environment.brdfLut->GetSampler()->Bind(static_cast<uint32_t>(TextureSlot::BRDF_LUT_TEXTURE_SLOT));
        skybox.environment.brdfLut->Bind(static_cast<uint32_t>(TextureSlot::BRDF_LUT_TEXTURE_SLOT));
        shader->SetUniform1i("u_BRDFLUT", static_cast<uint32_t>(TextureSlot::BRDF_LUT_TEXTURE_SLOT));

        sceneData.materialUniformBuffer->SetData(&materialData, sizeof(MaterialData));
        renderer->SubmitMesh(shader, mesh, transform.GetWorldTransformMatrix());
    }
    shader->UnBind();
    frameBuffer->UnBind();

    frameBuffer->Bind();
    sceneData.skyboxShader->Bind();
    skybox.environment.envMap->Bind(static_cast<uint32_t>(TextureSlot::SKYBOX_TEXTURE_SLOT));
    sceneData.skyboxShader->SetUniform1i("u_Skybox", static_cast<uint32_t>(TextureSlot::SKYBOX_TEXTURE_SLOT));
    const auto projectionMatrix = glm::perspective(glm::radians(45.0f),
                                                   16.0f / 9.0f,
                                                   0.1f,
                                                   300.0f);
    const auto viewMatrix = glm::mat4(glm::mat3(scene->GetCamera()->GetViewMatrix()));
    sceneData.skyboxShader->SetUniformMat4f("u_Projection", projectionMatrix);
    sceneData.skyboxShader->SetUniformMat4f("u_View", viewMatrix);
    auto& rendererSettings = renderer->GetSettings();
    rendererSettings.depthTestMode = RightEngine::DepthTestMode::LEQUAL;
    renderer->Configure();
    renderer->SubmitMesh(sceneData.skyboxShader,
                         sceneData.skyboxCube->GetComponent<Mesh>(),
                         sceneData.skyboxCube->GetComponent<Transform>().GetWorldTransformMatrix());
    rendererSettings.depthTestMode = RightEngine::DepthTestMode::LESS;
    renderer->Configure();
    frameBuffer->UnBind();
    renderer->EndScene();
}

void SandboxLayer::OnImGuiRender()
{
    bool dockspaceOpen = true;
    ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
    ImGui::PopStyleVar(3);

    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspaceID = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);
    }
    else
    {
        R_ASSERT(false, "");
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Options"))
        {
            if (ImGui::MenuItem("Exit"))
            {
                EventDispatcher::Get().Emit(ShutdownEvent());
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::Begin("Scene Hierarchy");
    if (ImGui::TreeNodeEx("Root", ImGuiTreeNodeFlags_OpenOnArrow))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 3);
        const auto& node = scene->GetRootNode();
        ImGuiAddTreeNodeChildren(node);
        ImGui::TreePop();
        ImGui::PopStyleVar();
    }
    ImGui::End();

    ImGui::Begin("Viewport");
    id = frameBuffer->GetColorAttachment();
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    if (viewportSize.x != sceneData.viewportSize.x || viewportSize.y != sceneData.viewportSize.y)
    {
        if (viewportSize.y <= 0)
        {
            viewportSize.y = 1;
        }
        // TODO: Fix image flickering while resizing window
        frameBuffer->Resize(static_cast<uint32_t>(viewportSize.x), static_cast<uint32_t>(viewportSize.y));
        sceneData.viewportSize = viewportSize;
    }
    ImGui::Image((void*) id, sceneData.viewportSize, ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();

    sceneData.propertyPanel.OnImGuiRender();

    ImGui::End();
}
