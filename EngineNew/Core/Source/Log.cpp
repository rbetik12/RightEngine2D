#include <Core/Log.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <sstream>
#include <iomanip>
#include <filesystem>

namespace core::log::impl
{
	constexpr const std::string_view C_LOG_DIR = "logs";
	constexpr const std::string_view C_LOG_FILENAME = "engine_log_";

	Logger::Logger()
	{
		const auto now = std::chrono::system_clock::now();
		const auto timestamp = std::chrono::system_clock::to_time_t(now);
		struct tm timeinfo;
#ifdef R_WIN32
		localtime_s(&timeinfo, &timestamp);
#else
		localtime_r(&timestamp, &timeinfo);
#endif

		const auto abs = std::filesystem::absolute(C_LOG_DIR);
		std::filesystem::create_directories(abs);

		std::stringstream filenameSS;
		filenameSS << C_LOG_DIR << "/" << C_LOG_FILENAME << std::put_time(&timeinfo, "%d-%m-%Y_%H-%M-%S") << ".log";
		const std::string logFilename = filenameSS.str();

		// Create a file sink with the generated log file name
		auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilename, true);

#ifdef R_WIN32
		auto winSink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
		s_instance = std::make_unique<spdlog::logger>("Engine", std::initializer_list<spdlog::sink_ptr>{ fileSink, winSink });
#else
		auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		m_loggerInstance = std::make_unique<spdlog::logger>("Engine", std::initializer_list<spdlog::sink_ptr>{ consoleSink, fileSink });
#endif

		// Set the log level
		s_instance->set_level(spdlog::level::debug);
		s_instance->flush_on(spdlog::level::debug);

		s_instance->info("[Logger] Logging was initialized. Logs directory: {}", abs.generic_u8string());
	}

	const std::unique_ptr<spdlog::logger>& Logger::GetLogger() const
	{
		return s_instance;
	}

	Logger& Logger::Get()
	{
		static Logger logger;
		return logger;
	}

	std::unique_ptr<spdlog::logger> Logger::s_instance;
}
