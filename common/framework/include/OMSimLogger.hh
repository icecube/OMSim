#ifndef OMSimLogger_h
#define OMSimLogger_h 1

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>
#include "spdlog/fmt/fmt.h" 

// Global logger instance
extern std::shared_ptr<spdlog::logger> global_logger;

// Custom logging function
void customLog(spdlog::level::level_enum log_level, const char* file, int line, const char* func, const std::string& message);

#define log_trace(...)   customLog(spdlog::level::trace, __FILE__, __LINE__, __func__, fmt::format(__VA_ARGS__))
#define log_debug(...)   customLog(spdlog::level::debug, __FILE__, __LINE__, __func__, fmt::format(__VA_ARGS__))
#define log_info(...)    customLog(spdlog::level::info, __FILE__, __LINE__, __func__, fmt::format(__VA_ARGS__))
#define log_notice(...)    customLog(spdlog::level::info, __FILE__, __LINE__, __func__, fmt::format(__VA_ARGS__))
#define log_warning(...)    customLog(spdlog::level::warn, __FILE__, __LINE__, __func__, fmt::format(__VA_ARGS__))
#define log_error(...)   customLog(spdlog::level::err, __FILE__, __LINE__, __func__, fmt::format(__VA_ARGS__))
#define log_critical(...) customLog(spdlog::level::critical, __FILE__, __LINE__, __func__, fmt::format(__VA_ARGS__))

#endif
//