#ifndef COBALT_SIMD_MATH_MAT3_H
#define COBALT_SIMD_MATH_MAT3_H

#include "simd_vec3.h"

namespace cobalt::simd {

struct mat3f {
    vec3f columns[3];

    mat3f(float diagonal)
        : columns{
              {diagonal,      0.f,      0.f},
              {     0.f, diagonal,      0.f},
              {     0.f,      0.f, diagonal}
    } {};

    mat3f(const vec3f &column1, const vec3f &column2, const vec3f &column3): columns{column1, column2, column3} {
    }

    vec3f &operator[](size_t idx) {
        return columns[idx];
    }

    friend vec3f operator*(const mat3f &a, vec3f b) {
        const std::array<float, 4> bValues = b.Values();
        vec3f out = a.columns[0] * bValues[0];
        out += a.columns[1] * bValues[1];
        out += a.columns[2] * bValues[2];
        return out;
    }

    friend mat3f operator*(const mat3f &a, const mat3f &b) {
        mat3f out(0.f);
        out.columns[0] = a * b.columns[0];
        out.columns[1] = a * b.columns[1];
        out.columns[2] = a * b.columns[2];
        return out;
    }

    friend mat3f operator+(const mat3f &a, const mat3f &b) {
        mat3f out(0.f);
        out.columns[0] = a.columns[0] + b.columns[0];
        out.columns[1] = a.columns[1] + b.columns[1];
        out.columns[2] = a.columns[2] + b.columns[2];
        return out;
    }

    friend mat3f operator-(const mat3f &a, const mat3f &b) {
        mat3f out(0.f);
        out.columns[0] = a.columns[0] - b.columns[0];
        out.columns[1] = a.columns[1] - b.columns[1];
        out.columns[2] = a.columns[2] - b.columns[2];
        return out;
    }

    friend mat3f operator*(const mat3f &a, float b) {
        mat3f out(0.f);
        out.columns[0] = a.columns[0] * b;
        out.columns[1] = a.columns[1] * b;
        out.columns[2] = a.columns[2] * b;
        return out;
    }

    friend mat3f operator*(float a, const mat3f &b) {
        return b * a;
    }

    friend bool operator==(const mat3f &a, const mat3f &b) {
        return a.columns[0] == b.columns[0] && a.columns[1] == b.columns[1] && a.columns[2] == b.columns[2];
    }
};

} // namespace cobalt::simd

#endif // COBALT_SIMD_MATH_MAT3_H
