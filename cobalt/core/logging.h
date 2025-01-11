#ifndef CBLT_LOGGING_H
#define CBLT_LOGGING_H

#include <iostream>
#include <memory>
#include <ostream>

enum CoLogLevel {
#ifdef CBLT_LOG_DEBUG
    CoLogLevelDebug,
#endif
#ifdef CBLT_LOG_INFO
    CoLogLevelInfo,
#endif
#ifdef CBLT_LOG_WARN
    CoLogLevelWarn,
#endif
#ifdef CBLT_LOG_ERROR
    CoLogLevelError
#endif
};

using CoLogStream = std::ostream;

class CoLog {
    public:
        const char *logName;

        CoLog(const char *name): logName{name} {};

    private:
};

#define CBLT_DEFINE_LOG(NAME) static CoLog NAME = CoLog(#NAME);

CoLogStream &CoLogWrite(CoLogLevel level, CoLog &log);

#ifdef CBLT_LOG_DEBUG
#define CoLogDebug(log) CoLogWrite(CoLogLevelDebug, log)
#endif // CBLT_LOG_DEBUG

#ifdef CBLT_LOG_INFO
#define CoLogInfo(log) CoLogWrite(CoLogLevelInfo, log)
#endif // CBLT_LOG_INFO

#ifdef CBLT_LOG_WARN
#define CoLogWarning(log) CoLogWrite(CoLogLevelWarning, log)
#endif // CBLT_LOG_WARN

#ifdef CBLT_LOG_ERROR
#define CoLogError(log) CoLogWrite(CoLogLevelError, log)
#endif // CBLT_LOG_ERROR

#endif // CBLT_LOGGING_H
