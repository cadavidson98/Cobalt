#ifndef COBALT_CORE_CALLBACK_h
#define COBALT_CORE_CALLBACK_h

#include "size_types.h"

#include <functional>
#include <mutex>

namespace cobalt::core {

struct Callback {
    mutable std::mutex mutex;
    mutable uint32_t totalProgress = 0;
    std::function<void(const char *, int)> functor;
    void pump(const char *status, uint32_t incrementalProgress = 0) const {
        std::lock_guard<std::mutex> lock(mutex);
        totalProgress += incrementalProgress;
        functor(status, totalProgress);
    }
};

} // namespace cobalt::core

#endif // COBALT_CORE_CALLBACK_H
