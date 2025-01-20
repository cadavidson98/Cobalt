#ifndef CBLT_LOGGING_H
#define CBLT_LOGGING_H

#include <iostream>
#include <memory>
#include <ostream>

namespace cblt::core {

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

void CoLogWrite(CoLogLevel, const char *trace, const char *message, ...);

}  // namespace cblt::core

#ifdef CBLT_LOG_DEBUG
#define CoLogDebug(...) cblt::core::CoLogWrite(cblt::core::CoLogLevelDebug, __PRETTY_FUNCTION__, __VA_ARGS__)
#endif // CBLT_LOG_DEBUG

#ifdef CBLT_LOG_INFO
#define CoLogInfo(...) cblt::core::CoLogWrite(cblt::core::CoLogLevelInfo, __PRETTY_FUNCTION__, __VA_ARGS__)
#endif // CBLT_LOG_INFO

#ifdef CBLT_LOG_WARN
#define CoLogWarning(...) cblt::core::CoLogWrite(cblt::core::CoLogLevelWarn, __PRETTY_FUNCTION__, __VA_ARGS__)
#endif // CBLT_LOG_WARN

#ifdef CBLT_LOG_ERROR
#define CoLogError(...) cblt::core::CoLogWrite(cblt::core::CoLogLevelError, __PRETTY_FUNCTION__, __VA_ARGS__)
#endif // CBLT_LOG_ERROR

#endif // CBLT_LOGGING_H
