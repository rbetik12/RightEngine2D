#pragma once

#include <Core/Log.hpp>

namespace rhi::log
{
    constexpr const std::string_view C_RHI_LOG_PREFIX = "[RHI] ";

    template<typename... Args>
    void debug(std::string_view message, Args... args)
    {
        core::log::debug(std::string(C_RHI_LOG_PREFIX).append(message), std::forward<Args>(args)...);
    }

    template<typename... Args>
    void info(std::string_view message, Args... args)
    {
        core::log::info(std::string(C_RHI_LOG_PREFIX).append(message), std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warning(std::string_view message, Args... args)
    {
        core::log::warning(std::string(C_RHI_LOG_PREFIX).append(message), std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(std::string_view message, Args... args)
    {
        core::log::error(std::string(C_RHI_LOG_PREFIX).append(message), std::forward<Args>(args)...);
    }
}