#ifndef COBALT_MATH_MAT3_H
#define COBALT_MATH_MAT3_H

#include "vec3.h"

namespace cobalt {

struct mat3f;

constexpr vec3f operator*(const mat3f &a, vec3f b);

constexpr mat3f operator*(const mat3f &a, const mat3f &b);

constexpr mat3f operator+(const mat3f &a, const mat3f &b);

constexpr mat3f operator-(const mat3f &a, const mat3f &b);

constexpr mat3f operator*(const mat3f &a, float b);

constexpr mat3f operator*(float a, const mat3f &b);

struct mat3f {
    vec3f columns[3];

    constexpr mat3f(float diagonal = 1.f)
        : columns{
              {.x = diagonal,      .y = 0.f,      .z = 0.f},
              {     .x = 0.f, .y = diagonal,      .z = 0.f},
              {     .x = 0.f,      .y = 0.f, .z = diagonal}
    } {};

    constexpr mat3f(const vec3f &diagonal)
        : columns{
              {.x = diagonal.x,        .y = 0.f,        .z = 0.f},
              {       .x = 0.f, .y = diagonal.y,        .z = 0.f},
              {       .x = 0.f,        .y = 0.f, .z = diagonal.z}
    } {
    }

    constexpr mat3f(const vec3f &column1, const vec3f &column2, const vec3f &column3)
        : columns{column1, column2, column3} {
    }

    constexpr vec3f &operator[](size_t idx) {
        return columns[idx];
    }

    constexpr const vec3f &operator[](size_t idx) const {
        return columns[idx];
    }

    constexpr friend vec3f operator*(const mat3f &a, vec3f b);

    constexpr friend mat3f operator*(const mat3f &a, const mat3f &b);

    constexpr friend mat3f operator+(const mat3f &a, const mat3f &b);

    constexpr friend mat3f operator-(const mat3f &a, const mat3f &b);

    constexpr friend mat3f operator*(const mat3f &a, float b);

    constexpr friend mat3f operator*(float a, const mat3f &b);
};

constexpr vec3f operator*(const mat3f &a, vec3f b) {
    vec3f out = a.columns[0] * b.x;
    out += a.columns[1] * b.y;
    out += a.columns[2] * b.z;
    return out;
}

constexpr mat3f operator*(const mat3f &a, const mat3f &b) {
    mat3f out(0.f);
    out.columns[0] = a * b.columns[0];
    out.columns[1] = a * b.columns[1];
    out.columns[2] = a * b.columns[2];
    return out;
}

constexpr mat3f operator+(const mat3f &a, const mat3f &b) {
    mat3f out(0.f);
    out.columns[0] = a.columns[0] + b.columns[0];
    out.columns[1] = a.columns[1] + b.columns[1];
    out.columns[2] = a.columns[2] + b.columns[2];
    return out;
}

constexpr mat3f operator-(const mat3f &a, const mat3f &b) {
    mat3f out(0.f);
    out.columns[0] = a.columns[0] - b.columns[0];
    out.columns[1] = a.columns[1] - b.columns[1];
    out.columns[2] = a.columns[2] - b.columns[2];
    return out;
}

constexpr mat3f operator*(const mat3f &a, float b) {
    mat3f out(0.f);
    out.columns[0] = a.columns[0] * b;
    out.columns[1] = a.columns[1] * b;
    out.columns[2] = a.columns[2] * b;
    return out;
}

constexpr mat3f operator*(float a, const mat3f &b) {
    return b * a;
}

struct Result {
    mat3f inverse;
    bool valid;
};

constexpr Result invert(const mat3f &matrix) {
    auto minorDeterminant = [](float a, float b, float c, float d) -> float {
        return a * b - c * d;
    };

    Result result;

    result.inverse[0].x = minorDeterminant(matrix[1].y, matrix[2].z, matrix[2].y, matrix[1].z);
    result.inverse[0].y = -minorDeterminant(matrix[0].y, matrix[2].z, matrix[2].y, matrix[0].z);
    result.inverse[0].z = minorDeterminant(matrix[0].y, matrix[1].z, matrix[1].y, matrix[0].z);

    const float determinant = matrix[0].x * result.inverse[0].x + matrix[1].x * result.inverse[0].y +
                              matrix[2].x * result.inverse[0].z;
    if (determinant == 0.f) {
        result.valid = false;
        return result;
    }

    result.valid = true;

    const float invDeterminant = 1.f / determinant;

    result.inverse[1].x = -minorDeterminant(matrix[1].x, matrix[2].z, matrix[2].x, matrix[1].z);
    result.inverse[1].y = minorDeterminant(matrix[0].x, matrix[2].z, matrix[2].x, matrix[0].z);
    result.inverse[1].z = -minorDeterminant(matrix[0].x, matrix[1].z, matrix[1].x, matrix[0].z);

    result.inverse[2].x = minorDeterminant(matrix[1].x, matrix[2].y, matrix[2].x, matrix[1].y);
    result.inverse[2].y = -minorDeterminant(matrix[0].x, matrix[2].y, matrix[2].x, matrix[0].y);
    result.inverse[2].z = minorDeterminant(matrix[0].x, matrix[1].y, matrix[1].x, matrix[0].y);

    result.inverse.columns[0] = result.inverse.columns[0] * invDeterminant;
    result.inverse.columns[1] = result.inverse.columns[1] * invDeterminant;
    result.inverse.columns[2] = result.inverse.columns[2] * invDeterminant;

    return result;
}

} // namespace cobalt

#endif // COBALT_MATH_MAT3_H
