#ifndef CBLT_SIMD_VEC4_H
#define CBLT_SIMD_VEC4_H

#include <immintrin.h>

namespace cblt::simd {

enum : unsigned int {
    x = 3 << 6,
    y = 2 << 4,
    z = 1 << 2,
    w = 0,
};

struct vec4f {
        union {
                __m128 xyzw;
                struct alignas(16) {
                        float x;
                        float y;
                        float z;
                        float w;
                };
        };

        vec4f(__m128 _xyzw) {
            xyzw = _xyzw;
        }

        vec4f(float x, float y, float z, float w) {
            xyzw = _mm_setr_ps(x, y, z, w);
        }

        vec4f &operator+=(const vec4f &rhs) {
            xyzw = _mm_add_ps(xyzw, rhs.xyzw);
            return *this;
        }

        vec4f &operator-=(const vec4f &rhs) {
            xyzw = _mm_sub_ps(xyzw, rhs.xyzw);
            return *this;
        }

        vec4f &operator*=(const vec4f &rhs) {
            xyzw = _mm_mul_ps(xyzw, rhs.xyzw);
            return *this;
        }

        vec4f &operator/=(const vec4f &rhs) {
            xyzw = _mm_div_ps(xyzw, rhs.xyzw);
            return *this;
        }

        friend vec4f operator+(vec4f lhs, const vec4f &rhs) {
            lhs += rhs;
            return lhs;
        }

        friend vec4f operator-(vec4f lhs, const vec4f &rhs) {
            lhs -= rhs;
            return lhs;
        }

        friend vec4f operator*(vec4f lhs, const vec4f &rhs) {
            lhs *= rhs;
            return lhs;
        }

        friend vec4f operator/(vec4f lhs, const vec4f &rhs) {
            lhs /= rhs;
            return lhs;
        }

        friend vec4f operator*(vec4f lhs, float rhs) {
            const __m128 scalar = _mm_set1_ps(rhs);
            return {_mm_mul_ps(lhs.xyzw, scalar)};
        }

        friend vec4f operator/(vec4f lhs, float rhs) {
            const __m128 scalar = _mm_set1_ps(rhs);
            return {_mm_div_ps(lhs.xyzw, scalar)};
        }

        friend vec4f operator*(float lhs, vec4f rhs) {
            const __m128 scalar = _mm_set1_ps(lhs);
            return {_mm_mul_ps(rhs.xyzw, scalar)};
        }

        friend bool operator==(const vec4f &a, const vec4f &b) {
            return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
        }

        friend vec4f min(const vec4f &lhs, const vec4f &rhs);

        friend vec4f max(const vec4f &lhs, const vec4f &rhs);

        friend float reduceMin(const vec4f &lhs);

        friend float reduceMax(const vec4f &lhs);
};

inline vec4f min(const vec4f &lhs, const vec4f &rhs) {
    return { _mm_min_ps(lhs.xyzw, rhs.xyzw) };
}

inline vec4f max(const vec4f &lhs, const vec4f &rhs) {
    return { _mm_max_ps(lhs.xyzw, rhs.xyzw) };
}

inline float reduceMin(const vec4f &lhs) {
            const __m128 shuffleLeft1 = _mm_shuffle_ps(lhs.xyzw, lhs.xyzw, _MM_SHUFFLE(2, 1, 0, 3));
            const __m128 shuffleMin1 = _mm_min_ps(lhs.xyzw, shuffleLeft1);
            const __m128 shuffleLeft2 = _mm_shuffle_ps(shuffleMin1, shuffleMin1, _MM_SHUFFLE(1, 0, 3, 2)); 
            const __m128 shuffleMin2 = _mm_min_ps(shuffleMin1, shuffleLeft2);
            return _mm_cvtss_f32(shuffleMin2);
}

inline float reduceMax(const vec4f &lhs) {
            const __m128 shuffleLeft1 = _mm_shuffle_ps(lhs.xyzw, lhs.xyzw, _MM_SHUFFLE(2, 1, 0, 3));
            const __m128 shuffleMax1 = _mm_max_ps(lhs.xyzw, shuffleLeft1);
            const __m128 shuffleLeft2 = _mm_shuffle_ps(shuffleMax1, shuffleMax1, _MM_SHUFFLE(1, 0, 3, 2)); 
            const __m128 shuffleMax2 = _mm_max_ps(shuffleMax1, shuffleLeft2);
            return _mm_cvtss_f32(shuffleMax2);
}

} // namespace cblt::simd

#endif // CBLT_SIMD_VEC4_H
