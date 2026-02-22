#ifndef CBLT_LOGGING_H
#define CBLT_LOGGING_H

#include <iostream>
#include <memory>
#include <ostream>

namespace cblt::core {

enum class Log {
#ifdef CBLT_LOG_DEBUG
    kDebug,
#endif
#ifdef CBLT_LOG_INFO
    kInfo,
#endif
#ifdef CBLT_LOG_WARN
    kWarn,
#endif
#ifdef CBLT_LOG_ERROR
    kError
#endif
};

void LogWrite(Log log, const char *trace, const char *message, ...);

} // namespace cblt::core

#ifdef CBLT_LOG_DEBUG
#define CoLogDebug(...) cblt::core::LogWrite(cblt::core::Log::kDebug, __PRETTY_FUNCTION__, __VA_ARGS__)
#endif // CBLT_LOG_DEBUG

#ifdef CBLT_LOG_INFO
#define CoLogInfo(...) cblt::core::LogWrite(cblt::core::Log::kInfo, __PRETTY_FUNCTION__, __VA_ARGS__)
#endif // CBLT_LOG_INFO

#ifdef CBLT_LOG_WARN
#define CoLogWarning(...) cblt::core::LogWrite(cblt::core::Log::kWarn, __PRETTY_FUNCTION__, __VA_ARGS__)
#endif // CBLT_LOG_WARN

#ifdef CBLT_LOG_ERROR
#define CoLogError(...) cblt::core::LogWrite(cblt::core::Log::kError, __PRETTY_FUNCTION__, __VA_ARGS__)
#endif // CBLT_LOG_ERROR

#endif // CBLT_LOGGING_H
