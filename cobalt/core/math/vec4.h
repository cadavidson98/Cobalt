#ifndef CBLT_VEC4_H
#define CBLT_VEC4_H

#include <cstdint>

namespace cblt {
struct vec4f {
        float x;
        float y;
        float z;
        float w;

        friend vec4f operator*(const vec4f &lhs, float rhs) {
            return {lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs};
        }
};

struct vec4u8 {
        uint8_t x;
        uint8_t y;
        uint8_t z;
        uint8_t w;
};
} // namespace cblt

#endif // CBLT_VEC4_H
