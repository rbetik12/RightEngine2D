#pragma once

#include <Core/Config.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace core::log
{

namespace impl
{

class CORE_API Logger
{
public:
    Logger();

    inline const std::unique_ptr<spdlog::logger>& GetLogger() const;

    static Logger& Get();

private:
    static std::unique_ptr<spdlog::logger> loggerInstance;
};

}

template<typename... Args>
void debug(std::string_view message, Args... args)
{
    impl::Logger::Get().GetLogger()->debug(message, std::forward<Args>(args)...);
}

template<typename... Args>
void info(std::string_view message, Args... args)
{
    impl::Logger::Get().GetLogger()->info(message, std::forward<Args>(args)...);
}

template<typename... Args>
void warning(std::string_view message, Args... args)
{
    impl::Logger::Get().GetLogger()->warn(message, std::forward<Args>(args)...);
}

template<typename... Args>
void error(std::string_view message, Args... args)
{
    impl::Logger::Get().GetLogger()->error(message, std::forward<Args>(args)...);
}

}
