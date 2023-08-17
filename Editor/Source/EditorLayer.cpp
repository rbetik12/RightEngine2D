#include "EditorLayer.hpp"
#include "Renderer.hpp"
#include "RendererCommand.hpp"
#include "Entity.hpp"
#include "Components.hpp"
#include "TextureLoader.hpp"
#include "Panels/PropertyPanel.hpp"
#include "EnvironmentMapLoader.hpp"
#include "MeshLoader.hpp"
#include "AssetManager.hpp"
#include "KeyCodes.hpp"
#include "SceneRenderer.hpp"
#include "MouseEvent.hpp"
#include "SceneSerializer.hpp"
#include "Filesystem.hpp"
#include "EditorCore.hpp"
#include "ImGuiLayer.hpp"
#include "ThreadService.hpp"
#include "Panels/ContentBrowserPanel.hpp"
#include "Timer.hpp"
#include "Service/SelectionService.hpp"
#include <RHI/ShaderCompiler.hpp>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

using namespace RightEngine;
using namespace editor;

namespace
{
    enum class GeometryType
    {
        NONE,
        CUBE,
        PLANE
    };

    const uint32_t width = 1280;
    const uint32_t height = 720;

    struct LightOrtho
    {
        float nearPlane = 1.0f;
        float farPlane = 7.5f;
        float left = -10.0f, bottom = -10.0f;
        float right = 10.0f, top = 10.0f;
    };

    struct LayerSceneData
    {
        ImVec2 viewportSize{ width, height };
        ImVec2 newViewportSize{ 0, 0 };
        std::shared_ptr<ImGuiLayer> imGuiLayer;
        std::shared_ptr<SceneRenderer> renderer;
        SceneRendererSettings rendererSettings;
        bool isViewportHovered{ false };
        ImGuizmo::OPERATION gizmoType{ ImGuizmo::TRANSLATE };
        std::filesystem::path scenePath;
        bool showDemoWindow{ false };
        bool showRenderDebug{ false };
        LightOrtho lightOrtho;
    };

    LayerSceneData sceneData;

    void ImGuiAddTreeNodeChildren(const std::shared_ptr<Entity>& node, const std::shared_ptr<Scene>& scene)
    {
        auto& ss = Instance().Service<SelectionService>();
        for (const auto& entity: node->GetChildren())
        {
            const auto& tag = entity->GetComponent<TagComponent>();
            ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow;
            bool node_open = ImGui::TreeNodeEx((char*) tag.guid.str().c_str(), node_flags, "%s", tag.name.c_str());
            if (ImGui::IsItemClicked())
            {
                ss.Entity(entity);
            }

            bool destroyEntity = false;
            if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonRight))
            {
                if (ImGui::MenuItem("Destroy Entity"))
                {
                    destroyEntity = true;
                }
                ImGui::EndPopup();
            }

            if (node_open)
            {
                ImGuiAddTreeNodeChildren(entity, scene);
                ImGui::TreePop();
            }

            if (destroyEntity)
            {
                const auto children = entity->GetChildren();
                const auto parentNode = entity->GetParent();
                for (const auto& child: children)
                {
                    parentNode->AddChild(child);
                }

                scene->DestroyEntity(entity);
                parentNode->RemoveChild(entity);
                ss.Entity(nullptr);
                return;
            }
        }
    }
}

void EditorLayer::OpenScene(const fs::path& path)
{
    const auto scene = LoadScene(path);
    R_ASSERT(scene, "");
    sceneData.scenePath = path;
    Scene(scene);
}

void EditorLayer::Scene(const std::shared_ptr<RightEngine::Scene>& scene)
{
    m_newScene = scene;
    m_propertyPanel.SetScene(scene);
}

void EditorLayer::AddCommand(EditorCommand&& command)
{
    std::lock_guard l(m_editorCommandMutex);
    m_editorCommands.emplace_back(std::move(command));
}

void EditorLayer::NewScene()
{
    Scene(Scene::Create());
}

void EditorLayer::LoadDefaultScene()
{
    OpenScene(C_DEFAULT_SCENE_PATH);
}

std::shared_ptr<RightEngine::Scene> EditorLayer::LoadScene(const fs::path& path)
{
    fs::path scenePath = path;
    if (scenePath.is_relative())
    {
        scenePath = G_ASSET_DIR + path.generic_string();
    }
    if (!fs::exists(scenePath))
    {
        return nullptr;
    }
    SceneSerializer serializer(Scene::Create(true));
    R_CORE_ASSERT(serializer.Deserialize(scenePath), "");
    return serializer.GetScene();
}

#include <RHI/RHI.hpp>
#include <Core/Log.hpp>

void EditorLayer::OnAttach()
{
    auto compiler = rhi::ShaderCompiler::Create();
    compiler->Compile(Path::Absolute("/Engine/Shaders/test.glsl"));
    __debugbreak();

    RightEngine::Timer timer;
	m_propertyPanel.Init();
    sceneData.renderer = std::make_shared<SceneRenderer>();
    sceneData.renderer->Init();

    const auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow()->GetNativeHandle());
    int windowW, windowH;
    glfwGetFramebufferSize(window, &windowW, &windowH);
    EventDispatcher::Get().Subscribe(MouseMovedEvent::descriptor, EVENT_CALLBACK(EditorLayer::OnEvent));
    sceneData.imGuiLayer = std::make_shared<ImGuiLayer>(sceneData.renderer->GetPass(PassType::UI));
    Application::Get().PushOverlay(sceneData.imGuiLayer);
    sceneData.renderer->SetUIPassCallback([&](const std::shared_ptr<CommandBuffer>& cmd)
                                          {
                                              sceneData.imGuiLayer->Begin();
                                              OnImGuiRender();
                                              sceneData.imGuiLayer->End(cmd);
                                          });

    auto& ss = Instance().Service<SelectionService>();
    ss.SelectEntityCallback([=](std::shared_ptr<Entity> entity)
        {
            m_propertyPanel.SetSelectedEntity(entity);
        });
    ss.DeselectEntityCallback([=]()
        {
            m_propertyPanel.SetSelectedEntity(nullptr);
        });

    LoadDefaultScene();
    R_INFO("[EditorLayer] Layer was successfully attached for {}s", timer.TimeInSeconds());
}

void EditorLayer::OnUpdate(float ts)
{
    if (m_newScene)
    {
        m_scene = m_newScene;
        m_newScene = nullptr;
    }
    if (sceneData.newViewportSize.x != 0 && sceneData.newViewportSize.y != 0)
    {
        sceneData.renderer->Resize(sceneData.newViewportSize.x, sceneData.newViewportSize.y);
        sceneData.newViewportSize = { 0, 0 };
    }

    m_scene->OnUpdate(ts);

    CameraData cameraData{};
    for (const auto eCamera : m_scene->GetRegistry().view<CameraComponent>())
    {
        auto& camera = m_scene->GetRegistry().get<CameraComponent>(eCamera);
        auto& transform = m_scene->GetRegistry().get<TransformComponent>(eCamera);
        camera.Rotate(glm::degrees(transform.rotation));
        camera.OnUpdate(ts);
        if (camera.isPrimary)
        {
            camera.isActive = Input::IsMouseButtonDown(MouseButton::Right) && sceneData.isViewportHovered;
            if (camera.isActive)
            {
                glm::vec3 position = transform.position;
                if (Input::IsKeyDown(R_KEY_W))
                {
                    position = camera.Move(R_KEY_W, position);
                }
                if (Input::IsKeyDown(R_KEY_S))
                {
                    position = camera.Move(R_KEY_S, position);
                }
                if (Input::IsKeyDown(R_KEY_A))
                {
                    position = camera.Move(R_KEY_A, position);
                }
                if (Input::IsKeyDown(R_KEY_D))
                {
                    position = camera.Move(R_KEY_D, position);
                }
                transform.position = position;
            }
            cameraData.position = transform.GetWorldPosition();
            cameraData.view = camera.GetViewMatrix(cameraData.position);
            cameraData.projection = camera.GetProjectionMatrix();
        }
    }

    std::vector<LightData> lightData;
    for (const auto& entityID: m_scene->GetRegistry().view<LightComponent>())
    {
        const auto& transform = m_scene->GetRegistry().get<TransformComponent>(entityID);
        const auto& light = m_scene->GetRegistry().get<LightComponent>(entityID);
        LightData shaderLight{};
        shaderLight.type = static_cast<int>(light.type);
        shaderLight.position = glm::vec4(transform.GetWorldPosition(), 1);
        shaderLight.color = glm::vec4(light.color, 1);
        shaderLight.intensity = light.intensity;
        shaderLight.radiusInner = light.innerRadius;
        shaderLight.radiusOuter = light.outerRadius;
        shaderLight.rotation = glm::vec4(transform.rotation, 1);

        CameraComponent camera;
        camera.Rotate(shaderLight.rotation);
        float near_plane = sceneData.lightOrtho.nearPlane, far_plane = sceneData.lightOrtho.farPlane;
        auto lightProj = glm::ortho(sceneData.lightOrtho.left, sceneData.lightOrtho.right, sceneData.lightOrtho.bottom, sceneData.lightOrtho.top, near_plane, far_plane);
        lightProj[1][1] *= -1;
        // const auto lightView = camera.GetViewMatrix(shaderLight.position);
        const auto lightView = glm::lookAt(glm::vec3(shaderLight.position),glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        shaderLight.lightSpace = lightProj * lightView;

        lightData.emplace_back(shaderLight);
    }

    auto& assetManager = AssetManager::Get();

    AssetHandle environmentHandle;
    for (const auto& entityID: m_scene->GetRegistry().view<SkyboxComponent>())
    {
        R_CORE_ASSERT(!environmentHandle.guid.isValid(), "")
        const auto& skybox = m_scene->GetRegistry().get<SkyboxComponent>(entityID);
        environmentHandle = skybox.environmentHandle;
        R_CORE_ASSERT(environmentHandle.guid.isValid(), "")
    }

    sceneData.renderer->SetScene(m_scene);
    sceneData.renderer->BeginScene(cameraData,
                                   assetManager.GetAsset<EnvironmentContext>(environmentHandle),
                                   lightData,
                                   sceneData.rendererSettings);

    for (auto& entity: m_scene->GetRegistry().view<MeshComponent>())
    {
        const auto& tag = m_scene->GetRegistry().get<TagComponent>(entity);
        const auto& transform = m_scene->GetRegistry().get<TransformComponent>(entity);
        const auto& meshComponent = m_scene->GetRegistry().get<MeshComponent>(entity);
        const auto& materialRef = meshComponent.material;
        if (!meshComponent.isVisible)
        {
            continue;
        }

        sceneData.renderer->SubmitMeshNode(assetManager.GetAsset<MeshNode>(meshComponent.mesh),
                                           assetManager.GetAsset<Material>(materialRef),
                                           transform.GetWorldTransformMatrix());
    }

    sceneData.renderer->EndScene();


    std::vector<EditorCommand> queue;
    {
        std::lock_guard l(m_editorCommandMutex);
        queue = std::move(m_editorCommands);
    }

    for (auto&& command : queue)
    {
        command();
    }
}

void EditorLayer::OnImGuiRender()
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
    } else
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

        if (ImGui::BeginMenu("Scene"))
        {
            if (ImGui::MenuItem("Save"))
            {
                if (sceneData.scenePath.empty())
                {
                    R_WARN("Scene path wasn't set!");
                }
                SceneSerializer serializer(m_scene);
                serializer.Serialize(sceneData.scenePath);
            }
            if (ImGui::MenuItem("Save As"))
            {
                sceneData.scenePath = Filesystem::SaveFileDialog({ "All Files(*.*)", "*.*"});
                SceneSerializer serializer(m_scene);
                serializer.Serialize(sceneData.scenePath);
            }
            if (ImGui::MenuItem("Load"))
            {
                SceneSerializer serializer(Scene::Create());
                const auto path = Filesystem::OpenFileDialog({ ".yaml" });
                if (!path.empty())
                {
                    serializer.Deserialize(path.generic_u8string());
                    sceneData.scenePath = path;
                	m_newScene = serializer.GetScene();
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("New"))
            {
                NewScene();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools"))
        {
            if (ImGui::MenuItem("ImGui Demo window"))
            {
                sceneData.showDemoWindow = !sceneData.showDemoWindow;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Render debug"))
            {
                sceneData.showRenderDebug = !sceneData.showRenderDebug;
            }
            ImGui::EndMenu();
        }

        if (sceneData.showDemoWindow)
        {
            ImGui::ShowDemoWindow();
        }

        ImGui::EndMenuBar();
    }

    ImGui::Begin("Scene Hierarchy");
    if (ImGui::TreeNodeEx("Root", ImGuiTreeNodeFlags_OpenOnArrow))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 3);
        const auto& node = m_scene->GetRootNode();
        ImGuiAddTreeNodeChildren(node, m_scene);
        ImGui::TreePop();
        ImGui::PopStyleVar();
    }

    if (ImGui::BeginPopupContextWindow(nullptr, 1, false))
    {
        if (ImGui::MenuItem("Create Empty Entity"))
        {
            auto entity = m_scene->CreateEntity("New entity", true);
        }

        ImGui::EndPopup();
    }

    ImGui::End();

    ImGui::Begin("Viewport");
    if (sceneData.isViewportHovered && ImGui::IsMouseClicked(0))
    {
        auto pos = ImGui::GetMousePos();
        auto rectPos = ImGui::GetWindowPos();
        glm::vec2 pickPos = { pos.x - rectPos.x, pos.y - rectPos.y };
        AddCommand([this, pickPos]()
            {
                auto id = sceneData.renderer->Pick(m_scene, pickPos);
				const auto entities = m_scene->GetRootNode()->GetAllChildren();
                std::shared_ptr<Entity> selectedEntity;
				for (const auto& entity : entities)
				{
                    auto& tag = entity->GetComponent<TagComponent>();
                    if (tag.colorId == id)
                    {
                        selectedEntity = entity;
                        break;
                    }
				}
                auto& ss = Instance().Service<SelectionService>();
                ss.Entity(selectedEntity);
            });
    }
    sceneData.isViewportHovered = ImGui::IsWindowHovered();
    const auto attachment = sceneData.renderer->GetFinalImage();
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    if (viewportSize.x != sceneData.viewportSize.x || viewportSize.y != sceneData.viewportSize.y)
    {
        if (viewportSize.y <= 0)
        {
            viewportSize.y = 1;
        }
        // TODO: Fix image flickering while resizing window
        sceneData.newViewportSize = viewportSize;
        sceneData.viewportSize = viewportSize;
    }
    ImGuiLayer::Image(attachment, sceneData.viewportSize);
//    sceneData.camera->SetAspectRatio(viewportSize.x / viewportSize.y);

    if (ImGui::BeginDragDropTarget())
    {
        const auto payload = ImGui::AcceptDragDropPayload(C_CONTENT_BROWSER_DND_NAME);
        if (payload)
        {
            static char pathBuff[256]{};
            memset(pathBuff, 0, 256);
            memcpy(pathBuff, payload->Data, payload->DataSize);
            fs::path path = pathBuff;
            if (path.extension() == ".scene")
            {
                OpenScene(path);
            }
        }
        ImGui::EndDragDropTarget();
    }

    auto& ss = Instance().Service<SelectionService>();
    auto selectedEntity = ss.Entity();
    if (selectedEntity)
    {
        if (sceneData.isViewportHovered)
        {
            if (Input::IsKeyDown(R_KEY_T))
            {
                sceneData.gizmoType = ImGuizmo::TRANSLATE;
            } else if (Input::IsKeyDown(R_KEY_R))
            {
                sceneData.gizmoType = ImGuizmo::ROTATE;
            } else if (Input::IsKeyDown(R_KEY_B))
            {
                sceneData.gizmoType = ImGuizmo::SCALE;
            }
        }
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

        CameraComponent camera{};
        glm::vec3 cameraPos;
        for (const auto& eCamera : m_scene->GetRegistry().view<CameraComponent>())
        {
            auto& cameraComp = m_scene->GetRegistry().get<CameraComponent>(eCamera);
            auto& transform = m_scene->GetRegistry().get<TransformComponent>(eCamera);
            if (cameraComp.isPrimary)
            {
                camera = cameraComp;
                cameraPos = transform.GetWorldPosition();
                break;
            }
        }

        glm::mat4 cameraView = camera.GetViewMatrix(cameraPos);
        glm::mat4 cameraProjection = camera.GetProjectionMatrix();

        auto& entityTransformComponent = selectedEntity->GetComponent<TransformComponent>();
        glm::mat4 entityTransform = entityTransformComponent.GetWorldTransformMatrix();

        ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                             (ImGuizmo::OPERATION) sceneData.gizmoType, ImGuizmo::LOCAL, glm::value_ptr(entityTransform));


        if (ImGuizmo::IsUsing())
        {
            glm::vec3 position, rotation, scale;
            Utils::DecomposeTransform(entityTransform, position, rotation, scale);

            auto deltaRotation = rotation - entityTransformComponent.rotation;
            entityTransformComponent.position = position;
            entityTransformComponent.SetRotationRadians(entityTransformComponent.rotation + deltaRotation);
            entityTransformComponent.scale = scale;
        }
    }

    ImGui::End();

    m_propertyPanel.OnImGuiRender();

    ImGui::Begin("Renderer");
    ImGui::DragFloat("Gamma", &sceneData.rendererSettings.gamma, 0.1, 1.0, 3.2);
    ImGui::Separator();
    ImGui::Text("Frame time %.2f ms", Input::frameTime);

    ImGui::SliderFloat("left", &sceneData.lightOrtho.left, -20, 20);
    ImGui::SliderFloat("right", &sceneData.lightOrtho.right, -20, 20);
    ImGui::SliderFloat("top", &sceneData.lightOrtho.top, -20, 20);
    ImGui::SliderFloat("bottom", &sceneData.lightOrtho.bottom, -20, 20);
    ImGui::SliderFloat("near", &sceneData.lightOrtho.nearPlane, 1.0, 20);
    ImGui::SliderFloat("far", &sceneData.lightOrtho.farPlane, 1.0, 40);

    for (const auto& pass : sceneData.renderer->GetPassInfo())
    {
        ImGui::Text("%s: %.2fms", pass.m_name.c_str(), pass.m_time);
    }

    ImGui::End();

    m_contentBrowser.OnImGuiRender();

    if (sceneData.showRenderDebug)
    {
        m_renderDebugPanel.Renderer(sceneData.renderer);
        m_renderDebugPanel.OnImGuiRender();
    }

    ImGui::End();
}

bool EditorLayer::OnEvent(const Event& event)
{
    if (!m_scene)
    {
        return false;
    }
    if (event.GetType() == MouseMovedEvent::descriptor)
    {
        MouseMovedEvent mouseMovedEvent = static_cast<const MouseMovedEvent&>(event);
        for (const auto& eCamera : m_scene->GetRegistry().view<CameraComponent>())
        {
            auto& camera = m_scene->GetRegistry().get<CameraComponent>(eCamera);
            auto& transform = m_scene->GetRegistry().get<TransformComponent>(eCamera);
            if (camera.isPrimary)
            {
                auto rotation = camera.Rotate(mouseMovedEvent.GetX(), mouseMovedEvent.GetY(), glm::degrees(transform.rotation));
                transform.SetRotationRadians(glm::radians(rotation));
                break;
            }
        }
        return true;
    }

    return false;
}
