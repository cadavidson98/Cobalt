#ifndef CBLT_MATH_CONSTANTS_H
#define CBLT_MATH_CONSTANTS_H
#include "vec.h"
#include "mat4.h"

#include <cmath>
#include <limits>

namespace cblt
{
    // scalar constants
    const float inf_F = std::numeric_limits<float>::max();
    const float minus_inf_F = std::numeric_limits<float>::lowest();
    const float eps_zero_F = 1.e-6f;

    constexpr const float  PI_f = 3.1415926535897932384626433832795028841971f;
    constexpr const double PI_d = 3.1415926535897932384626433832795028841971;
    
    constexpr const float INV_PI_f = 1.f / PI_f;
    constexpr const double INV_PI_d = 1.f / PI_d;
    
    // vector constants
    const Vec3 X_axis_F = {1.f, 0.f, 0.f};
    const Vec3 Y_axis_F = {0.f, 1.f, 0.f};
    const Vec3 Z_axis_F = {0.f, 0.f, 1.f};

    // matrix constants
    const Mat4 Identity_F = Mat4(1.f);
}

#endif  // CBLT_MATH_CONSTANTS_H