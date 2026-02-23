#ifndef COBALT_SYSTEM_H
#define COBALT_SYSTEM_H

#include "size_types.h"

namespace cobalt::core {

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

} // namespace cobalt::core

#endif // COBALT_SYSTEM_H
