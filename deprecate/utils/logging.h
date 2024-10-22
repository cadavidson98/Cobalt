#ifndef COBALT_LOGGING_H
#define COBALT_LOGGING_H

namespace cblt
{
    enum class CoLogEvent
    {
        INTEGRATE_LIGHT,
        INTEGRATE_VOLUME,
        INTERSECTION,
        SAMPLE_BSDF
    };

    void CO_LOG_INFO(const char* msg);
    void CO_LOG_ERROR(const char* msg);
    void CO_LOG_EVENT(CoLogEvent path, const char* msg);
}

#endif  // COBALT_LOGGING_H