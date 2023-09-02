#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "Public/Logger.hpp"

namespace RightEngine
{
    std::shared_ptr<spdlog::logger> Log::clientLogger;
    std::shared_ptr<spdlog::logger> Log::coreLogger;
    bool Log::bIsInitialized = false;

    void Log::Init()
    {
        if (!bIsInitialized)
        {
            spdlog::set_pattern("%^[%T] %n: %v%$");
            coreLogger = spdlog::stdout_color_mt("Engine");
            coreLogger->set_level(spdlog::level::trace);
            clientLogger = spdlog::stderr_color_mt("Editor");
            clientLogger->set_level(spdlog::level::trace);
            bIsInitialized = true;
            R_CORE_INFO("Successfully initialized logging system!");
        } else
        {
            R_CORE_WARN("Logging system was already initialized!");
        }
    }

}