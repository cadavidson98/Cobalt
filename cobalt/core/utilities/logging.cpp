#ifndef CBLT_LOGGING_INL
#define CBLT_LOGGING_INL

#include "logging.h"

#include <format>
#include <iostream>
#include <string>

CoLogStream &CoLogWrite(CoLogLevel level, CoLog &log) {
    std::string logCategory;
    switch (level) {
#ifdef CBLT_LOG_DEBUG
    case CoLogLevelDebug : logCategory = "debug"; break;
#endif
#ifdef CBLT_LOG_INFO
    case CoLogLevelInfo : logCategory = "info"; break;
#endif
#ifdef CBLT_LOG_WARN
    case CoLogLevelWarn : logCategory = "warn"; break;
#endif
#ifdef CBLT_LOG_ERROR
    case CoLogLevelError : logCategory = "error"; break;
#endif
    };

    return std::cout << log.logName << ": " << logCategory << " ";
}

#endif // CBLT_LOGGING_INL
