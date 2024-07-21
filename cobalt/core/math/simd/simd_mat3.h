#ifndef CBLT_SIMD_MATH_MAT3_H
#define CBLT_SIMD_MATH_MAT3_H

#include "simd_vec3.h"

namespace cblt::simd {

struct mat3f {
        vec3f columns[3];

        mat3f(float diagonal): columns{{diagonal, 0.f, 0.f}, {0.f, diagonal, 0.f}, {0.f, 0.f, diagonal}} {};

        mat3f(const vec3f &diagonal): columns{{diagonal.x, 0.f, 0.f}, {0.f, diagonal.y, 0.f}, {0.f, 0.f, diagonal.z}} {
        }

        mat3f(const vec3f &column1, const vec3f &column2, const vec3f &column3): columns{column1, column2, column3} {
        }

        vec3f &operator[](size_t idx) {
            return columns[idx];
        }

        friend vec3f operator*(const mat3f &a, vec3f b) {
            vec3f out = a.columns[0] * b.x;
            out += a.columns[1] * b.y;
            out += a.columns[2] * b.z;
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
            return
                a.columns[0] == b.columns[0] &&
                a.columns[1] == b.columns[1] &&
                a.columns[2] == b.columns[2];
        }
};

} // namespace cblt::simd

#endif // CBLT_SIMD_MATH_MAT3_H
