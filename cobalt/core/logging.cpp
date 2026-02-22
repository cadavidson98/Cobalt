#include "logging.h"

#ifndef __linux__
#error Only Linux Supported
#endif

#include <cstdarg>
#include <cstring>

#include <syslog.h>

namespace cblt::core {

namespace {

class Logger {
public:
    Logger() {
        openlog("Cobalt", LOG_PID, LOG_USER);
    }

    ~Logger() {
        closelog();
    }

    void writeMessage(int level, const char *trace, const char *message) {
        syslog(level | LOG_USER, "%s: %s", trace, message);
    }

private:
};

std::unique_ptr<Logger> gLog = nullptr;

} // anonymous namespace

void LogWrite(Log level, const char *trace, const char *message, ...) {
    int osLogLevel = LOG_DEBUG;
    switch (level) {
#ifdef CBLT_LOG_DEBUG
    case Log::kDebug : {
        osLogLevel = LOG_DEBUG;
        break;
    }
#endif
#ifdef CBLT_LOG_INFO
    case Log::kInfo : {
        osLogLevel = LOG_INFO;
        break;
    }
#endif
#ifdef CBLT_LOG_WARN
    case Log::kWarn : {
        osLogLevel = LOG_WARNING;
        break;
    }
#endif
#ifdef CBLT_LOG_ERROR
    case Log::kError : {
        osLogLevel = LOG_ERR;
        break;
    }
#endif
    };

    if (!gLog) {
        gLog = std::make_unique<Logger>();
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
