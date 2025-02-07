#ifndef CBLT_CORE_CALLBACK_h
#define CBLT_CORE_CALLBACK_h

#include "size_types.h"

#include <atomic>
#include <functional>

namespace cblt::core {

struct CoCallback {
    std::mutex mutex;
    uint32_t totalProgress = 0;
    std::function<void(const char *, int)> functor;
    void pump(const char *status, uint32_t incrementalProgress = 0) {
        std::lock_guard<std::mutex> lock(mutex);
        totalProgress += incrementalProgress;
        functor(status, totalProgress);
    }
};

} // namespace cblt::core

#endif // CBLT_CORE_CALLBACK_h
