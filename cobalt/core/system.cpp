#include "system.h"

#include <time.h>

namespace cblt::core {

uint64_t time() {
    static constexpr uint64_t kSecondsToNanoSeconds = 1e9;
    timespec systemTime;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &systemTime);
    return (systemTime.tv_sec) * kSecondsToNanoSeconds + (systemTime.tv_nsec);
}

}  // namespace cblt