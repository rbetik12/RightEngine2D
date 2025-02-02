#pragma once

#include <Engine/Config.hpp>
#include <Engine/Assert.hpp>
#include <Engine/Timer.hpp>
#include <Engine/Service/ServiceManager.hpp>
#include <Core/Type.hpp>
#include <filesystem>

namespace engine
{

namespace io
{
namespace fs = std::filesystem;
} // io

class ENGINE_API Engine final : private core::NonCopyable
{
    friend Engine& Instance();
public:
    struct Config
    {
        io::fs::path    m_projectPath;
        Domain          m_domain = Domain::NONE;
    };

    Engine(int argCount, char* argPtr[]);
    ~Engine();

    int             Run();
    void            Stop();

    template<typename T>
    T&              Service() { return m_serviceManager->Service<T>(); }

    template<typename T>
    T*              FindService() { return m_serviceManager->FindService<T>(); }

    const Config&   Cfg() const { return m_config; }

private:
    void            Update();
    void            ParseCfg(int argCount, char* argPtr[]);

private:
    inline static Engine*               s_instance = nullptr;

    bool                                m_running = false;
    Timer                               m_timer;
    Timer                               m_frameLimiterTimer;
    std::unique_ptr<ServiceManager>     m_serviceManager;
    Config                              m_config;
};

ENGINE_FORCE_INLINE Engine& Instance()
{
    ENGINE_ASSERT(Engine::s_instance);
    return *Engine::s_instance;
}

} // engine