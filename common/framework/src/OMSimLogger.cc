#include "OMSimLogger.hh"


// Definition of the customLog function
void customLog(spdlog::level::level_enum pLoglevel, const char* pFile, int pLine, const char* pFunc, const std::string& pMessage) {
    if (g_logger && g_logger->should_log(pLoglevel)) {
        g_logger->log(spdlog::source_loc{pFile, pLine, pFunc}, pLoglevel, pMessage);
    }
}
