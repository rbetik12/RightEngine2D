#include "Application.hpp"
#include "Assert.hpp"
#include "Logger.hpp"
#include "RendererCommand.hpp"
#include "Input.hpp"
#include "ImGuiLayer.hpp"
#include "Core.hpp"
#include "Surface.hpp"
#include "AssetManager.hpp"
#include "Filesystem.hpp"
#include "MaterialLoader.hpp"
#include <memory>

namespace RightEngine
{
    Application& Application::Get()
    {
        static Application application;
        return application;
    }

    Application::Application()
    {}

    Application::~Application()
    {
        for (auto& layer: layers)
        {
            layer->OnDetach();
            layer.reset();
        }

        layers.clear();
    }

    void Application::Init()
    {
        window.reset(Window::Create("Right Editor", 1920, 1080));
        RendererCommand::Init(GGPU_API);

        auto& manager = AssetManager::Get();
        manager.RegisterLoader<TextureLoader>(std::make_shared<TextureLoader>());
        manager.RegisterLoader<EnvironmentMapLoader>(std::make_shared<EnvironmentMapLoader>());
        manager.RegisterLoader<MeshLoader>(std::make_shared<MeshLoader>());
        manager.RegisterLoader<MaterialLoader>(std::make_shared<MaterialLoader>());

        Filesystem::Init();

        static bool wasCalled = false;
        R_CORE_ASSERT(!wasCalled, "PostInit was called twice!");
        wasCalled = true;
        R_CORE_INFO("Successfully initialized application!");
    }

    void Application::OnUpdate()
    {
        Input::OnUpdate();
        window->OnUpdate();

        for (const auto& layer: layers)
        {
            layer->OnUpdate(Input::deltaTime);
        }
    }

    void Application::OnUpdateEnd()
    {
        window->Swap();
    }

    void Application::PushLayer(const std::shared_ptr<Layer>& layer)
    {
        // TODO: Move to LayerStack data structure
        static int layerIndex = 0;
        layers.emplace(layers.begin() + layerIndex, layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(const std::shared_ptr<Layer>& layer)
    {
        layers.emplace_back(layer);
        layer->OnAttach();
    }

    const std::shared_ptr<Window>& Application::GetWindow() const
    {
        return window;
    }

}
