#pragma once

#include <memory>
#include "spdlog/logger.h"

namespace RightEngine
{
    class Log
    {
    public:
        static void Init();

        inline static std::shared_ptr<spdlog::logger> &GetCoreLogger()
        { return coreLogger; }

        inline static std::shared_ptr<spdlog::logger> &GetClientLogger()
        { return clientLogger; }
    private:
        static std::shared_ptr<spdlog::logger> coreLogger;
        static std::shared_ptr<spdlog::logger> clientLogger;
        static bool bIsInitialized;
    };
}

#ifdef R_TEST_BUILD
#define R_CORE_TRACE(...)
#define R_CORE_INFO(...)
#define R_CORE_WARN(...)
#define R_CORE_ERROR(...)
#define R_CORE_FATAL(...)

#define R_VULKAN_LOG(message)

#define R_TRACE(...)
#define R_INFO(...)
#define R_WARN(...)
#define R_ERROR(...)
#define R_FATAL(...)
#else
//Core log macros
#define R_CORE_TRACE(...) ::RightEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define R_CORE_INFO(...)  ::RightEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define R_CORE_WARN(...)  ::RightEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define R_CORE_ERROR(...) ::RightEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define R_CORE_FATAL(...) ::RightEngine::Log::GetCoreLogger()->fatal(__VA_ARGS__)

#define R_VULKAN_LOG(message) ("[Vulkan]: " + std::string(message))

//Client log macros
#define R_TRACE(...)      ::RightEngine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define R_INFO(...)       ::RightEngine::Log::GetClientLogger()->info(__VA_ARGS__)
#define R_WARN(...)       ::RightEngine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define R_ERROR(...)      ::RightEngine::Log::GetClientLogger()->error(__VA_ARGS__)
#define R_FATAL(...)      ::RightEngine::Log::GetClientLogger()->fatal(__VA_ARGS__)
#endif