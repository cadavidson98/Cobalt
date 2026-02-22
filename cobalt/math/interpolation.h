#ifndef CBLT_MATH_INTERPOLATION_H
#define CBLT_MATH_INTERPOLATION_H

#include "vec2.h"
#include "vec3.h"

#include <cmath>

namespace cblt::math {

template<typename T>
T bilinearInterpolation(const T &a, const T &b, const T &c, const T &d, vec2f weights) {
    const T ab = std::lerp(a, b, weights.x);
    const T cd = std::lerp(c, d, weights.x);

    return std::lerp(ab, cd, weights.y);
}

template<typename T>
T barycentricInterpolation(const T &a, const T &b, const T &c, vec3f weights) {
    return a * weights.x + b * weights.y + c * weights.z;
}

} // namespace cblt::math

#endif // CBLT_MATH_INTERPOLATION_H
