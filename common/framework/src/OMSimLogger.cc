#include "OMSimLogger.hh"


// Definition of the customLog function
void customLog(spdlog::level::level_enum pLoglevel, const char* pFile, int pLine, const char* pFunc, const std::string& pMessage) {
    if (globalLogger && globalLogger->should_log(pLoglevel)) {
        globalLogger->log(spdlog::source_loc{pFile, pLine, pFunc}, pLoglevel, pMessage);
    }
}
