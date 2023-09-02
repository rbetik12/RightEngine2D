#include "ShutdownEvent.hpp"
#include "Logger.hpp"
#include "LaunchEngine.hpp"
#include "EventDispatcher.hpp"
#include "Input.hpp"
#include "Assert.hpp"

bool GShouldStop = true;
LaunchMode GLaunchMode = LaunchMode::Game;
RightEngine::GPU_API GGPU_API = RightEngine::GPU_API::None;
std::string G_ENGINE_ASSET_DIR = ENGINE_ASSETS_DIR;

namespace RightEngine
{
    void LaunchEngine::Init(int argc, char* argv[])
    {
        Log::Init();
        Input::Init();
        launchContext = new LaunchContext();
        launchContext->SetCmdArgs(argc, argv);
        GShouldStop = false;
    }

    void LaunchEngine::Exit()
    {
        R_CORE_INFO("Exiting engine!");

        delete launchContext;
    }

    LaunchEngine::LaunchContext::LaunchContext()
    {
        EventDispatcher::Get().Subscribe(ShutdownEvent::descriptor, EVENT_CALLBACK(LaunchEngine::LaunchContext::OnEvent));
    }

    void LaunchEngine::LaunchContext::SetCmdArgs(int argc, char **argv)
    {
        easyArgs = std::make_unique<EasyArgs>(argc, argv);
        easyArgs->Version("0.0.1");
        easyArgs->Value("-m", "--mode", "Engine launch modes [Game|Test].", false);
        easyArgs->Value("-g", "--gpu-api", "Engine rendering backend [Vulkan].", false);

        ParseCmdArgs();
    }

    void LaunchEngine::LaunchContext::ParseCmdArgs()
    {
        std::string mode = easyArgs->GetValueFor("-m");
        if (mode == "Game")
        {
            GLaunchMode = LaunchMode::Game;
        }
        else if (mode == "Test")
        {
            GLaunchMode = LaunchMode::Test;
        }

        std::string gpuApi = easyArgs->GetValueFor("--gpu-api");
        GGPU_API = GPU_API::Vulkan;
        if (gpuApi == "OpenGL")
        {
            R_CORE_WARN("OpenGL is deprecated!");
            GGPU_API = GPU_API::OpenGL;
        }
        else
        {
            R_CORE_WARN("No GPU API was provided in cmd!");
        }
    }

    bool LaunchEngine::LaunchContext::OnEvent(const Event& event)
    {
        if (event.GetType() == ShutdownEvent::descriptor)
        {
            GShouldStop = true;
        }

        return true;
    }

    LaunchEngine::LaunchContext *LaunchEngine::launchContext = nullptr;
}