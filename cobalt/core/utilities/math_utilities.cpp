#include "math_utilities.h"

#include <cmath>

namespace cblt::utils {

mat4f perspectiveProjection(float nearPlane, float farPlane, float hFov, float vFov) {
    const float halfWidth = std::tan(hFov * 0.5f);
    const float halfHeight = std::tan(vFov * 0.5f);
    const float depthRange = farPlane - nearPlane;

    return mat4f{
        vec4f(1.f / halfWidth, 0.f, 0.f, 0.f),
        vec4f(0.f, 1.f / halfHeight, 0.f, 0.f),
        vec4f(0.f, 0.f, farPlane / depthRange, 1.f),
        vec4f(0.f, 0.f, -(nearPlane * farPlane) / depthRange, 0.f),
    };
}

mat4f perspectiveProjectionInv(float nearPlane, float farPlane, float hFov, float vFov) {
    const float halfWidth = std::tan(hFov * 0.5f);
    const float halfHeight = std::tan(vFov * 0.5f);
    const float depthRange = farPlane - nearPlane;

    return mat4f{
        vec4f(halfWidth, 0.f, 0.f, 0.f),
        vec4f(0.f, halfHeight, 0.f, 0.f),
        vec4f(0.f, 0.f, 0.f, depthRange / (-nearPlane * farPlane)),
        vec4f(0.f, 0.f, 1.f, 1.f / nearPlane),
    };
}

mat4f scaleMatrix(vec3f scale) {
    return mat4f{
        vec4f{scale.x, 0.f, 0.f, 0.f},
        vec4f{0.f, scale.y, 0.f, 0.f},
        vec4f{0.f, 0.f, scale.z, 0.f},
        vec4f{0.f, 0.f, 0.f, 1.f},
    };
}

mat4f translationMatrix(vec3f translation) {
    return mat4f{
        vec4f{1.f, 0.f, 0.f, 0.f},
        vec4f{0.f, 1.f, 0.f, 0.f},
        vec4f{0.f, 0.f, 1.f, 0.f},
        vec4f{translation.x, translation.y, translation.z, 1.f},
    };
}

} // namespace cblt::utils
