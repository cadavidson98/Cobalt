#ifndef CBLT_SIMD_VEC3_H
#define CBLT_SIMD_VEC3_H

#include <array>
#include <cassert>
#include <immintrin.h>
#include <limits>

namespace cblt::simd {

struct vec3f {
        __m128 xyz;
        vec3f(float c = 0.f) {
            xyz = _mm_setr_ps(c, c, c, 0.f);
        }

        vec3f(__m128 _xyz) {
            xyz = _xyz;
        }

        vec3f(float x, float y, float z) {
            xyz = _mm_setr_ps(x, y, z, 0.f);
        }

        std::array<float, 4> Values() const {
            std::array<float, 4> values;
            _mm_store_ps(values.data(), xyz);
            return values;
        };

        float operator[](const size_t idx) const {
            assert(idx < 3);
            std::array<float, 4> values = Values();
            return values[idx];
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
            const std::array<float, 4> lhsValues = a.Values();
            const std::array<float, 4> rhsValues = b.Values();
            return lhsValues[0] == rhsValues[0] && lhsValues[1] == rhsValues[1] && lhsValues[2] == rhsValues[2];
        }

        friend float dot(const vec3f &lhs, const vec3f &rhs);
        friend vec3f cross(const vec3f &lhs, const vec3f &rhs);
        friend vec3f lerp(const vec3f &lhs, const vec3f &rhs, const float value);

        friend vec3f min(const vec3f &lhs, const vec3f &rhs);
        friend vec3f max(const vec3f &lhs, const vec3f &rhs);

        friend float reduceMin(const vec3f &lhs);
        friend float reduceMax(const vec3f &lhs);

        friend vec3f abs(const vec3f &lhs);

        friend vec3f shuffle(const vec3f lhs, size_t idx1, size_t idx2, size_t idx3);
};

inline float dot(const vec3f &lhs, const vec3f &rhs) {
    __m128 product = _mm_mul_ps(lhs.xyz, rhs.xyz);
    product = _mm_hadd_ps(product, product);
    product = _mm_hadd_ps(product, product);
    return _mm_cvtss_f32(product);
}

inline vec3f cross(const vec3f &lhs, const vec3f &rhs) {
    const __m128 shuffleLhs = _mm_shuffle_ps(lhs.xyz, lhs.xyz, _MM_SHUFFLE(3, 0, 2, 1));
    const __m128 shuffleRhs = _mm_shuffle_ps(rhs.xyz, rhs.xyz, _MM_SHUFFLE(3, 0, 2, 1));
    const __m128 crossProduct = _mm_sub_ps(_mm_mul_ps(lhs.xyz, shuffleRhs), _mm_mul_ps(shuffleLhs, rhs.xyz));
    const __m128 deShuffleCross = _mm_shuffle_ps(crossProduct, crossProduct, _MM_SHUFFLE(3, 0, 2, 1));
    return {deShuffleCross};
};

inline vec3f lerp(const vec3f &lhs, const vec3f &rhs, const float value) {
    return lhs * (1.f - value) + rhs * value;
}

inline vec3f min(const vec3f &lhs, const vec3f &rhs) {
    return {_mm_min_ps(lhs.xyz, rhs.xyz)};
}

inline vec3f max(const vec3f &lhs, const vec3f &rhs) {
    return {_mm_max_ps(lhs.xyz, rhs.xyz)};
}

inline float reduceMin(const vec3f &lhs) {
    // mask out NaN -> replace with +inf
    static constexpr float kMaxFloat = std::numeric_limits<float>::max();
    static const __m128 kMaxInf = {kMaxFloat, kMaxFloat, kMaxFloat, kMaxFloat};
    __m128 maskNAN = _mm_cmp_ps(lhs.xyz, kMaxInf, _CMP_LT_OQ);
    __m128 values = _mm_blendv_ps(kMaxInf, lhs.xyz, maskNAN);
    __m128 shuffleLeft = _mm_shuffle_ps(lhs.xyz, lhs.xyz, _MM_SHUFFLE(2, 1, 0, 0));
    __m128 shuffleMin = _mm_min_ps(lhs.xyz, shuffleLeft);
    shuffleLeft = _mm_shuffle_ps(shuffleMin, shuffleMin, _MM_SHUFFLE(1, 0, 2, 2));
    shuffleMin = _mm_min_ps(shuffleMin, shuffleLeft);
    return _mm_cvtss_f32(shuffleMin);
}

inline float reduceMax(const vec3f &lhs) {
    // mask out NaN -> replace with -inf
    static constexpr float kMinFloat = std::numeric_limits<float>::lowest();
    static const __m128 kMinusInf = {kMinFloat, kMinFloat, kMinFloat, kMinFloat};
    __m128 maskNAN = _mm_cmp_ps(lhs.xyz, kMinusInf, _CMP_GT_OQ);
    __m128 values = _mm_blendv_ps(kMinusInf, lhs.xyz, maskNAN);
    __m128 shuffleLeft = _mm_shuffle_ps(values, values, _MM_SHUFFLE(2, 1, 0, 0));
    __m128 shuffleMax = _mm_max_ps(values, shuffleLeft);
    shuffleLeft = _mm_shuffle_ps(shuffleMax, shuffleMax, _MM_SHUFFLE(1, 0, 2, 2));
    shuffleMax = _mm_max_ps(shuffleMax, shuffleLeft);
    return _mm_cvtss_f32(shuffleMax);
}

inline vec3f abs(const vec3f &lhs) {
    static const __m128 kMinusZero = {-1.f, -1.f, -1.f, -1.f};
    return {_mm_andnot_ps(kMinusZero, lhs.xyz)};
}

inline vec3f shuffle(const vec3f lhs, size_t idx1, size_t idx2, size_t idx3) {
    float array[4];
    _mm_store_ps(array, lhs.xyz);
    float array2[] = {array[idx1], array[idx2], array[idx3], array[3]};
    return {_mm_load_ps(array2)};
}

} // namespace cblt::simd

#endif // CBLT_SIMD_VEC3_H
