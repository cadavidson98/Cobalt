#include "logging.h"

#ifndef __linux__
#error Only Linux Supported
#endif

#include <cstdarg>
#include <cstring>
#include <iostream>
#include <string>
#include <syslog.h>

namespace cblt::core {

class CoLogInternal {
public:
    CoLogInternal() {
        openlog("Cobalt", LOG_PID, LOG_USER);
    }

    ~CoLogInternal() {
        closelog();
    }

    void writeMessage(int level, const char *trace, const char *message) {
        syslog(level | LOG_USER, "%s: %s", trace, message);
    }

private:
};

static std::unique_ptr<CoLogInternal> gLog = nullptr;

void CoLogWrite(CoLogLevel level, const char *trace, const char *message, ...) {
    int osLogLevel = LOG_DEBUG;
    switch (level) {
#ifdef CBLT_LOG_DEBUG
    case CoLogLevelDebug : {
        osLogLevel = LOG_DEBUG;
        break;
    }
#endif
#ifdef CBLT_LOG_INFO
    case CoLogLevelInfo : {
        osLogLevel = LOG_INFO;
        break;
    }
#endif
#ifdef CBLT_LOG_WARN
    case CoLogLevelWarn : {
        osLogLevel = LOG_WARNING;
        break;
    }
#endif
#ifdef CBLT_LOG_ERROR
    case CoLogLevelError : {
        osLogLevel = LOG_ERR;
        break;
    }
#endif
    };

    if (!gLog) {
        gLog = std::make_unique<CoLogInternal>();
    }

    char logMessage[256];
    std::memset(logMessage, 0, 256);
    std::va_list messageArguments;
    va_start(messageArguments, message);
    std::vsnprintf(logMessage, 256, message, messageArguments);
    va_end(messageArguments);
    gLog->writeMessage(osLogLevel, trace, logMessage);
}

} // namespace cblt::core
