#ifndef COBALT_LOGGING_H
#define COBALT_LOGGING_H

#include <iostream>
#include <memory>
#include <ostream>

namespace cobalt::core {

enum class Log {
#ifdef COBALT_LOG_DEBUG
    kDebug,
#endif
#ifdef COBALT_LOG_INFO
    kInfo,
#endif
#ifdef COBALT_LOG_WARN
    kWarn,
#endif
#ifdef COBALT_LOG_ERROR
    kError
#endif
};

void LogWrite(Log log, const char *trace, const char *message, ...);

} // namespace cobalt::core

#ifdef COBALT_LOG_DEBUG
#define CoLogDebug(...) cobalt::core::LogWrite(cobalt::core::Log::kDebug, __PRETTY_FUNCTION__, __VA_ARGS__)
#endif // COBALT_LOG_DEBUG

#ifdef COBALT_LOG_INFO
#define CoLogInfo(...) cobalt::core::LogWrite(cobalt::core::Log::kInfo, __PRETTY_FUNCTION__, __VA_ARGS__)
#endif // COBALT_LOG_INFO

#ifdef COBALT_LOG_WARN
#define CoLogWarning(...) cobalt::core::LogWrite(cobalt::core::Log::kWarn, __PRETTY_FUNCTION__, __VA_ARGS__)
#endif // COBALT_LOG_WARN

#ifdef COBALT_LOG_ERROR
#define CoLogError(...) cobalt::core::LogWrite(cobalt::core::Log::kError, __PRETTY_FUNCTION__, __VA_ARGS__)
#endif // COBALT_LOG_ERROR

#endif // COBALT_LOGGING_H
