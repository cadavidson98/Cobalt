#ifndef CBLT_SYSTEM_H
#define CBLT_SYSTEM_H

#include "size_types.h"

namespace cblt::core {

uint64_t time();

struct DateTime {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

DateTime dateAndTime();

} // namespace cblt::core

#endif // CBLT_SYSTEM_H
