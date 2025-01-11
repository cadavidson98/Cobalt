#ifndef CBLT_SIMD_MATH_MAT4_H
#define CBLT_SIMD_MATH_MAT4_H

#include "simd_vec4.h"

namespace cblt::simd {

struct mat4f {
        vec4f columns[4];

        mat4f(float diagonal)
            : columns{
                  {diagonal, 0.f, 0.f, 0.f},
                  {0.f, diagonal, 0.f, 0.f},
                  {0.f, 0.f, diagonal, 0.f},
                  {0.f, 0.f, 0.f, diagonal}
              } {};

        mat4f(const vec4f &diagonal)
            : columns{
                  {diagonal.x, 0.f, 0.f, 0.f},
                  {0.f, diagonal.y, 0.f, 0.f},
                  {0.f, 0.f, diagonal.z, 0.f},
                  {0.f, 0.f, 0.f, diagonal.w}
              } {
        }

        mat4f(const vec4f &column1, const vec4f &column2, const vec4f &column3, const vec4f &column4)
            : columns{column1, column2, column3, column4} {
        }

        vec4f &operator[](size_t idx) {
            return columns[idx];
        }

        const vec4f &operator[](size_t idx) const {
            return columns[idx];
        }

        friend vec4f operator*(const mat4f &a, vec4f b) {
            vec4f out = a.columns[0] * b.x;
            out += a.columns[1] * b.y;
            out += a.columns[2] * b.z;
            out += a.columns[3] * b.w;
            return out;
        }

        friend mat4f operator*(const mat4f &a, const mat4f &b) {
            mat4f out(0.f);
            out.columns[0] = a * b.columns[0];
            out.columns[1] = a * b.columns[1];
            out.columns[2] = a * b.columns[2];
            out.columns[3] = a * b.columns[3];
            return out;
        }

        friend mat4f operator+(const mat4f &a, const mat4f &b) {
            mat4f out(0.f);
            out.columns[0] = a.columns[0] + b.columns[0];
            out.columns[1] = a.columns[1] + b.columns[1];
            out.columns[2] = a.columns[2] + b.columns[2];
            out.columns[3] = a.columns[3] + b.columns[3];
            return out;
        }

        friend mat4f operator-(const mat4f &a, const mat4f &b) {
            mat4f out(0.f);
            out.columns[0] = a.columns[0] - b.columns[0];
            out.columns[1] = a.columns[1] - b.columns[1];
            out.columns[2] = a.columns[2] - b.columns[2];
            out.columns[3] = a.columns[3] - b.columns[3];
            return out;
        }

        friend mat4f operator*(const mat4f &a, float b) {
            mat4f out(0.f);
            out.columns[0] = a.columns[0] * b;
            out.columns[1] = a.columns[1] * b;
            out.columns[2] = a.columns[2] * b;
            out.columns[3] = a.columns[3] * b;
            return out;
        }

        friend mat4f operator*(float a, const mat4f &b) {
            return b * a;
        }

        friend bool operator==(const mat4f &a, const mat4f &b) {
            return a.columns[0] == b.columns[0] && a.columns[1] == b.columns[1] && a.columns[2] == b.columns[2] &&
                   a.columns[3] == b.columns[3];
        }
};

} // namespace cblt::simd

#endif // CBLT_SIMD_MATH_MAT4_H
