#include "logging.h"
#include <iostream>

namespace cblt
{
    namespace
    {
        std::string toString(CoLogEvent event)
        {
            switch (event)
            {
            case CoLogEvent::INTEGRATE_LIGHT:
                return "INTEGRATE_LIGHT";
            case CoLogEvent::INTEGRATE_VOLUME:
                return "INTEGRATE_VOLUME";
            case CoLogEvent::INTERSECTION:
                return "INTERSECTION";
            case CoLogEvent::SAMPLE_BSDF:
                return "SAMPLE_BSDF";
            }
        }
    }

    void CO_LOG_INFO(const char* msg)
    {
        std::clog << "COBALT INFO: " << msg << '\n';
    }

    void CO_LOG_ERROR(const char* msg)
    {
        std::clog << "COBALT DEBUG: " << msg << '\n';
    }

    void CO_LOG_EVENT(CoLogEvent pathEvent, const char* msg)
    {
        std::clog << "COBALT EVENT " << toString(pathEvent) << ' ' << msg << '\n';
    }
}  // namespace cblt