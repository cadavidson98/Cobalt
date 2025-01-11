#ifndef CBLT_CORE_MATH_UTILITIES
#define CBLT_CORE_MATH_UTILITIES

#include "mat4.h"
#include "vec3.h"

namespace cblt::utils {
mat4f perspectiveProjection(float nearPlane, float farPlane, float hFov, float vFov);
mat4f perspectiveProjectionInv(float nearPlane, float farPlane, float hFov, float vFov);

mat4f scaleMatrix(vec3f scale);
mat4f translationMatrix(vec3f translation);

} // namespace cblt::utils

#endif // CBLT_CORE_MATH_UTILITIES
