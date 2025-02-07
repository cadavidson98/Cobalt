#ifndef CBLT_SIMD_MATH_VEC_H
#define CBLT_SIMD_MATH_VEC_H

#include <immintrin.h>

namespace cblt::math::simd {

struct vec3f {

    vec3f(__m128 xyz);
    vec3f(float x, float y, float z);

    union {
        __m128 xyz;
        struct alignas(16) {
            float x;
            float y;
            float z;
        };
    };

    friend vec3f operator+(vec3f a, const vec3f &b);
    friend vec3f operator-(vec3f a, const vec3f &b);
    friend vec3f operator*(vec3f a, const vec3f &b);

    vec3f &operator+=(const vec3f &b);
    vec3f &operator-=(const vec3f &b);

    friend vec3f operator*(vec3f a, float b);
    friend vec3f operator/(vec3f a, float b);

    friend vec3f operator*(float a, vec3f b);

    friend float dot(const vec3f &a, const vec3f &b);
    friend vec3f cross(const vec3f &a, const vec3f &b);
};

} // namespace cblt::math::simd
#endif // CBLT_SIMD_MATH_VEC_H
