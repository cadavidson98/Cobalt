#ifndef CBLT_CORE_MATH_UTILITIES
#define CBLT_CORE_MATH_UTILITIES

#include "constants.h"
#include "mat4.h"
#include "vec3.h"

namespace cblt::utils {

template<typename T>
constexpr T sqr(const T value) {
    return value * value;
}

template<typename T>
constexpr T divUp(const T lhs, T rhs) {
    return (lhs + rhs - T(1)) / rhs;
}

template<typename T>
constexpr T toRadians(const T degrees) {
    return degrees * T(kPI) / T(180);
}

template<typename T>
constexpr T toDegrees(const T radians) {
    return radians * T(180) / T(kPI);
}

mat4f perspectiveProjection(float nearPlane, float farPlane, float hFov, float vFov);
mat4f perspectiveProjectionInv(float nearPlane, float farPlane, float hFov, float vFov);

mat4f scaleMatrix(vec3f scale);
mat4f translationMatrix(vec3f translation);
mat4f rotationMatrix(vec3f axis, float angle);

} // namespace cblt::utils

#endif // CBLT_CORE_MATH_UTILITIES
