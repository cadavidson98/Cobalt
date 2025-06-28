#ifndef CBLT_CORE_MORTON_ENCODING_H
#define CBLT_CORE_MORTON_ENCODING_H

#include "size_types.h"

namespace cblt::core {

// Morton Encoding adopted from 'Physically Based Rendering: From Theory to Implementation' 4th Edition
// Assumes that x, y, z are all in the range 0, (1^10 - 1)
constexpr inline uint32_t mortonEncode(const uint32_t x, const uint32_t y, const uint32_t z) {

    auto shift = [](const uint32_t v) -> uint32_t {
        uint32_t value = v == 1 << 10 ? -v : v;
        value = (value | (value << 16)) & 0b00000011000000000000000011111111;
        value = (value | (value << 8)) & 0b00000011000000001111000000001111;
        value = (value | (value << 4)) & 0b00000011000011000011000011000011;
        value = (value | (value << 2)) & 0b00001001001001001001001001001001;
        return value;
    };

    return (shift(z) << 2) | (shift(y) << 1) | shift(x);
}

}  // namespace cblt::core

#endif  // CBLT_CORE_MORTON_ENCODING_H