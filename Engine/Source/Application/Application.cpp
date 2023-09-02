#include "Application.hpp"
#include "Assert.hpp"
#include "Logger.hpp"
#include "RendererCommand.hpp"
#include "Input.hpp"
#include "ImGuiLayer.hpp"
#include "Core.hpp"
#include "AssetManager.hpp"
#include "Filesystem.hpp"
#include "MaterialLoader.hpp"
#include <Engine/Service/Render/RenderService.hpp>
#include <Engine/Service/ThreadService.hpp>
#include "TextureLoader.hpp"
#include <memory>

namespace RightEngine
{
    Application& Instance()
    {
        return Application::Get();
    }

    Application& Application::Get()
    {
        static Application application;
        return application;
    }

    Application::Application()
    {}

    Application::~Application()
    {
        for (auto& layer: m_layers)
        {
            layer->OnDetach();
            layer.reset();
        }

        m_layers.clear();
    }

    void Application::Init()
    {
        RegisterService<engine::ThreadService>();
        RegisterService<engine::render::RenderService>();
        m_window.reset(Window::Create("Right Editor", 1920, 1080));
        RendererCommand::Init(GGPU_API);

        auto& manager = AssetManager::Get();
        manager.RegisterLoader<TextureLoader>(std::make_shared<TextureLoader>());
        manager.RegisterLoader<EnvironmentMapLoader>(std::make_shared<EnvironmentMapLoader>());
        manager.RegisterLoader<MeshLoader>(std::make_shared<MeshLoader>());
        manager.RegisterLoader<MaterialLoader>(std::make_shared<MaterialLoader>());

        Filesystem::Init();
        Path::Init();

        static bool wasCalled = false;
        R_CORE_ASSERT(!wasCalled, "PostInit was called twice!");
        wasCalled = true;
        R_CORE_INFO("Successfully initialized application!");
    }

    void Application::OnUpdate()
    {
        Input::OnUpdate();
        m_window->OnUpdate();

        for (const auto& layer: m_layers)
        {
            layer->OnUpdate(Input::deltaTime);
        }
    }

    void Application::OnUpdateEnd()
    {
        m_window->Swap();
    }

    void Application::PushLayer(const std::shared_ptr<Layer>& layer)
    {
        // TODO: Move to LayerStack data structure
        static int layerIndex = 0;
        m_layers.emplace(m_layers.begin() + layerIndex, layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(const std::shared_ptr<Layer>& layer)
    {
        m_layers.emplace_back(layer);
        layer->OnAttach();
    }

    const std::shared_ptr<Window>& Application::GetWindow() const
    {
        return m_window;
    }

}
