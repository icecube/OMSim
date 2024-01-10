#include "OMSimLogger.hh"


// Definition of the customLog function
void customLog(spdlog::level::level_enum log_level, const char* file, int line, const char* func, const std::string& message) {
    if (global_logger && global_logger->should_log(log_level)) {
        global_logger->log(spdlog::source_loc{file, line, func}, log_level, message);
    }
}
