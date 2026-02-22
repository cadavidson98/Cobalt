#include "system.h"

#include <cassert>

#include <time.h>

namespace cblt::core {

uint64_t time() {
    static constexpr uint64_t kSecondsToNanoSeconds = 1e9;
    timespec systemTime;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &systemTime);
    return (systemTime.tv_sec) * kSecondsToNanoSeconds + (systemTime.tv_nsec);
}

DateTime dateAndTime() {
    timespec posixTime;
    clock_gettime(CLOCK_REALTIME, &posixTime);

    tm *posixDateTime = gmtime(&posixTime.tv_sec);
    assert(posixDateTime && "date time must be not null");
    return DateTime{
        .year = uint16_t(posixDateTime->tm_year + 1900),
        .month = uint8_t(posixDateTime->tm_mon),
        .day = uint8_t(posixDateTime->tm_mday),
        .hour = uint8_t(posixDateTime->tm_hour),
        .minute = uint8_t(posixDateTime->tm_min),
        .second = uint8_t(posixDateTime->tm_sec),
    };
}

} // namespace cblt::core
