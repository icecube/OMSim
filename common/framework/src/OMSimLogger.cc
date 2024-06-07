#include "OMSimLogger.hh"


// Definition of the customLog function
void customLog(spdlog::level::level_enum log_level, const char* file, int line, const char* func, const std::string& message) {
    if (globalLogger && globalLogger->should_log(log_level)) {
        globalLogger->log(spdlog::source_loc{file, line, func}, log_level, message);
    }
}
