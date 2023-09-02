#pragma once

#include "Shader.hpp"
#include "Window.hpp"
#include "Scene.hpp"
#include "Layer.hpp"
#include <Engine/Service/IService.hpp>
#include <memory>
#include <typeindex>

namespace RightEngine
{
    class Application
    {
    public:
        static Application& Get();

        void OnUpdate();
        void OnUpdateEnd();

        // TODO: Delete layers
        void PushLayer(const std::shared_ptr<Layer>& layer);
        void PushOverlay(const std::shared_ptr<Layer>& layer);

        template<class T>
        void RegisterService();

        template<class T>
        T& Service();

        const std::shared_ptr<Window>& GetWindow() const;

        void Init();

        Application(const Application& other) = delete;
        Application& operator=(const Application& other) = delete;
        Application(Application&& other) = delete;
        Application& operator=(Application&& other) = delete;

    private:
        std::shared_ptr<Window> m_window;
        std::vector<std::shared_ptr<Layer>> m_layers;
        std::unordered_map<std::type_index, std::shared_ptr<engine::IService>> m_services;

    private:
        Application();
        ~Application();
    };

    Application& Instance();

    template <class T>
    void Application::RegisterService()
    {
        R_CORE_ASSERT(static_cast<bool>(std::is_base_of_v<engine::IService, T>), "");
        const auto typeIndex = std::type_index(typeid(T));
        const auto serviceIt = m_services.find(typeIndex);
        if (serviceIt == m_services.end())
        {
            auto service = std::make_shared<T>();
            m_services[typeIndex] = service;
            service->OnRegister();
            return;
        }
        R_CORE_ASSERT(false, "")
    }

    template <class T>
    T& Application::Service()
    {
        R_CORE_ASSERT(static_cast<bool>(std::is_base_of_v<engine::IService, T>), "");
        const auto typeIndex = std::type_index(typeid(T));
        const auto serviceIt = m_services.find(typeIndex);
        if (serviceIt == m_services.end())
        {
            static std::shared_ptr<T> empty;
            R_CORE_ASSERT(false, "");
            return *empty;
        }
        auto* s = static_cast<T*>(serviceIt->second.get());
        return *s;
    }
}
