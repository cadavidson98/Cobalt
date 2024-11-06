#ifndef CBLT_SIMD_VEC3_H
#define CBLT_SIMD_VEC3_H

#include <cassert>
#include <immintrin.h>

namespace cblt::simd {

struct vec3f {
        union {
                __m128 xyz;
                struct alignas(16) {
                        float x;
                        float y;
                        float z;
                };
        };

        vec3f(__m128 _xyz) {
            xyz = _xyz;
        }

        vec3f(float x, float y, float z) {
            xyz = _mm_setr_ps(x, y, z, 0.f);
        }

        float operator[](const size_t idx) const {
            assert(idx < 3);
            return *(&x + idx);
        }

        vec3f &operator+=(const vec3f &b) {
            xyz = _mm_add_ps(xyz, b.xyz);
            return *this;
        }

        vec3f &operator-=(const vec3f &b) {
            xyz = _mm_sub_ps(xyz, b.xyz);
            return *this;
        }

        vec3f &operator*=(const vec3f &rhs) {
            xyz = _mm_mul_ps(xyz, rhs.xyz);
            return *this;
        }

        vec3f &operator/=(const vec3f &rhs) {
            xyz = _mm_div_ps(xyz, rhs.xyz);
            return *this;
        }

        friend vec3f operator+(vec3f a, const vec3f &b) {
            a += b;
            return a;
        }

        friend vec3f operator-(vec3f a, const vec3f &b) {
            a -= b;
            return a;
        }

        friend vec3f operator*(vec3f a, const vec3f &b) {
            a *= b;
            return a;
        }

        friend vec3f operator/(vec3f a, const vec3f b) {
            a /= b;
            return a;
        }

        friend vec3f operator*(vec3f a, float b) {
            __m128 scalar = _mm_set1_ps(b);
            return {_mm_mul_ps(a.xyz, scalar)};
        }

        friend vec3f operator/(vec3f a, float b) {
            __m128 scalar = _mm_set1_ps(b);
            return {_mm_div_ps(a.xyz, scalar)};
        }

        friend vec3f operator*(float a, vec3f b) {
            __m128 scalar = _mm_set1_ps(a);
            return {_mm_mul_ps(b.xyz, scalar)};
        }

        friend bool operator==(const vec3f &a, const vec3f &b) {
            return a.x == b.x && a.y == b.y && a.z == b.z;
        }

        friend float dot(const vec3f &lhs, const vec3f &rhs);

        friend vec3f min(const vec3f &lhs, const vec3f &rhs);
        friend vec3f max(const vec3f &lhs, const vec3f &rhs);

        friend float reduceMin(const vec3f &lhs);
        friend float reduceMax(const vec3f &lhs);
};

inline float dot(const vec3f &lhs, const vec3f &rhs) {
    __m128 product = _mm_mul_ps(lhs.xyz, rhs.xyz);
    product = _mm_hadd_ps(product, product);
    product = _mm_hadd_ps(product, product);
    return _mm_cvtss_f32(product);
}

inline vec3f min(const vec3f &lhs, const vec3f &rhs) {
    return {_mm_min_ps(lhs.xyz, rhs.xyz)};
}

inline vec3f max(const vec3f &lhs, const vec3f &rhs) {
    return {_mm_max_ps(lhs.xyz, rhs.xyz)};
}

inline float reduceMin(const vec3f &lhs) {
    __m128 shuffleLeft = _mm_shuffle_ps(lhs.xyz, lhs.xyz, _MM_SHUFFLE(2, 1, 0, 0));
    __m128 shuffleMin = _mm_min_ps(lhs.xyz, shuffleLeft);
    shuffleLeft = _mm_shuffle_ps(shuffleMin, shuffleMin, _MM_SHUFFLE(1, 0, 2, 2));
    shuffleMin = _mm_min_ps(shuffleMin, shuffleLeft);
    return _mm_cvtss_f32(shuffleMin);
}

inline float reduceMax(const vec3f &lhs) {
    __m128 shuffleLeft = _mm_shuffle_ps(lhs.xyz, lhs.xyz, _MM_SHUFFLE(2, 1, 0, 0));
    __m128 shuffleMax = _mm_max_ps(lhs.xyz, shuffleLeft);
    shuffleLeft = _mm_shuffle_ps(shuffleMax, shuffleMax, _MM_SHUFFLE(1, 0, 2, 2));
    shuffleMax = _mm_max_ps(shuffleMax, shuffleLeft);
    return _mm_cvtss_f32(shuffleMax);
}

} // namespace cblt::simd

#endif // CBLT_SIMD_VEC3_H
