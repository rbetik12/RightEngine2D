#include <Engine/Engine.hpp>
#include <Engine/Domain.hpp>
#include <Engine/Service/WindowService.hpp>
#include <Engine/Service/ThreadService.hpp>
#include <Engine/Service/EditorService.hpp>
#include <Engine/Service/Render/RenderService.hpp>
#include <Engine/Service/ImGui/ImguiService.hpp>
#include <Engine/Service/Filesystem/VirtualFilesystemService.hpp>
#include <Engine/Service/Project/ProjectService.hpp>
#include <Engine/Service/WorldService.hpp>
#include <Engine/Service/Resource/ResourceService.hpp>
#include <Engine/Service/Resource/TextureResource.hpp>
#include <Core/Profiling.hpp>

RTTR_REGISTRATION
{
using namespace engine::registration;
using namespace rttr;

CommandLineArgs()
            .Argument(
                CommandLineArg("-m", "--mode")
                .Help("Engine launch mode - client, server, server_no_ui or editor")
                .DefaultValue("editor")
            );


registration::enumeration<engine::Domain>("engine::Domain")(
        value("none", engine::Domain::NONE),
        value("editor", engine::Domain::ALL),
        value("client", engine::Domain::CLIENT),
        value("server", engine::Domain::SERVER_UI),
        value("server_no_ui", engine::Domain::SERVER_NO_UI)
    );
}

namespace
{

constexpr float C_TARGET_FRAME_TIME = 16.6f;

} // unnamed

namespace engine
{

Engine::Engine(int argCount, char* argPtr[])
{
    m_timer.Restart();
    s_instance = this;

    ParseCfg(argCount, argPtr);

    core::log::info("Engine domain: {}", DomainToString(m_config.m_domain));

    m_serviceManager = std::make_unique<ServiceManager>(m_config.m_domain);

    m_serviceManager->RegisterService<ProjectService>();

    auto& ps = m_serviceManager->Service<ProjectService>();
    ps.Load(m_config.m_projectPath);

    m_serviceManager->RegisterService<io::VirtualFilesystemService>();

    auto& vfs = m_serviceManager->Service<io::VirtualFilesystemService>();
    for (auto& setting : ps.CurrentProject()->Setting<io::VFSSettings>().m_settings)
    {
        vfs.Assign(setting.m_alias, setting.m_path);
    }

    m_serviceManager->RegisterService<ThreadService>();
    m_serviceManager->RegisterService<WindowService>();
    m_serviceManager->RegisterService<RenderService>();

    m_serviceManager->RegisterService<ResourceService>();

    auto& rs = m_serviceManager->Service<ResourceService>();
    rs.RegisterLoader<TextureLoader>();

    m_serviceManager->RegisterService<ImguiService>();
    m_serviceManager->RegisterService<EditorService>();
    m_serviceManager->RegisterService<WorldService>();

    m_running = true;

    m_serviceManager->UpdateDependencyOrder();

    m_serviceManager->Service<EditorService>().Initialize();

    core::log::info("Frame limiter targeted frametime is {}ms", C_TARGET_FRAME_TIME);
    core::log::info("Engine was initialized successfully for {}s", m_timer.TimeInSeconds());
}

Engine::~Engine()
{
    m_serviceManager->Destroy();
    core::log::info("Engine was destroyed successfully");
}

int Engine::Run()
{
    core::log::info("Starting engine loop");

    m_timer.Restart();
    while (m_running)
    {
        Update();
        PROFILER_FRAME_END;
    }

    core::log::info("Stopped engine loop");
    return 0;
}

void Engine::Stop()
{
    core::log::info("Engine loop stop requested");
    m_running = false;
}

void Engine::Update()
{
    PROFILER_CPU_ZONE;
    m_timer.Stop();

    const float dt = std::max<float>(0.0001f, m_timer.TimeInMilliseconds() * 0.001f);
    m_timer.Start();

    m_frameLimiterTimer.Restart();

    m_serviceManager->Update(dt);
    m_serviceManager->PostUpdate(dt);

    {
        // TODO: Add more configuration for more stable frame rate
        PROFILER_CPU_ZONE_NAME("Framelimiter wait");
        float deltaWait = C_TARGET_FRAME_TIME - m_frameLimiterTimer.TimeInMilliseconds();

        if (deltaWait > 0)
        {
            deltaWait /= 2;
            const auto microseconds = std::chrono::microseconds(static_cast<long long>(deltaWait * 1000));
            std::this_thread::sleep_for(microseconds);
        }
    }
}

void Engine::ParseCfg(int argCount, char* argPtr[])
{
    registration::CommandLineArgs::Parse(argCount, argPtr);

    const auto launchMode = registration::CommandLineArgs::Get("--mode");
    m_config.m_domain = rttr::type::get_by_name("engine::Domain")
                                .get_enumeration()
                                .name_to_value(launchMode.data())
                                .get_value<Domain>();

    const io::fs::path projectPath = registration::CommandLineArgs::Get("--project");
    if (projectPath.is_absolute())
    {
        m_config.m_projectPath = projectPath;
    }
    else
    {
        m_config.m_projectPath = io::fs::absolute(projectPath);
    }
}

} // namespace engine

