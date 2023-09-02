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
		//CORE_ASSERT(res);

		std::stringstream filename_ss;
		filename_ss << C_LOG_DIR << "/" << C_LOG_FILENAME << std::put_time(&timeinfo, "%d-%m-%Y_%H-%M-%S") << ".log";
		const std::string log_filename = filename_ss.str();

		// Create a stdout sink
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

		// Create a file sink with the generated log file name
		auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_filename, true);

		// Create a logger with both sinks
		loggerInstance = std::make_unique<spdlog::logger>("Engine", std::initializer_list<spdlog::sink_ptr>{ console_sink, file_sink });

		// Set the log level
		loggerInstance->set_level(spdlog::level::debug);
		loggerInstance->flush_on(spdlog::level::debug);

		loggerInstance->info("[Logger] Logging was initialized. Logs directory: {}", abs.generic_u8string());
	}

	const std::unique_ptr<spdlog::logger>& Logger::GetLogger() const
	{
		return loggerInstance;
	}

	Logger& Logger::Get()
	{
		static Logger logger;
		return logger;
	}

	std::unique_ptr<spdlog::logger> Logger::loggerInstance;
}
