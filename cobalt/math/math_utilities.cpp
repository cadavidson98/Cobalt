#include "math_utilities.h"

#include <cmath>

namespace cobalt::utils {

mat4f perspectiveProjection(float nearPlane, float farPlane, float hFov, float vFov) {
    const float halfWidth = std::tan(hFov * 0.5f);
    const float halfHeight = std::tan(vFov * 0.5f);
    const float depthRange = farPlane - nearPlane;

    return mat4f{
        {1.f / halfWidth,              0.f,                                  0.f, 0.f},
        {            0.f, 1.f / halfHeight,                                  0.f, 0.f},
        {            0.f,              0.f,                farPlane / depthRange, 1.f},
        {            0.f,              0.f, -(nearPlane * farPlane) / depthRange, 0.f},
    };
}

mat4f perspectiveProjectionInv(float nearPlane, float farPlane, float hFov, float vFov) {
    const float halfWidth = std::tan(hFov * 0.5f);
    const float halfHeight = std::tan(vFov * 0.5f);
    const float depthRange = farPlane - nearPlane;

    return mat4f{
        {halfWidth,        0.f, 0.f,                                  0.f},
        {      0.f, halfHeight, 0.f,                                  0.f},
        {      0.f,        0.f, 0.f, depthRange / (-nearPlane * farPlane)},
        {      0.f,        0.f, 1.f,                      1.f / nearPlane},
    };
}

mat4f scaleMatrix(vec3f scale) {
    return mat4f{
        vec4f{scale.x,     0.f,     0.f, 0.f},
        vec4f{    0.f, scale.y,     0.f, 0.f},
        vec4f{    0.f,     0.f, scale.z, 0.f},
        vec4f{    0.f,     0.f,     0.f, 1.f},
    };
}

mat4f translationMatrix(vec3f translation) {
    return mat4f{
        vec4f{          1.f,           0.f,           0.f, 0.f},
        vec4f{          0.f,           1.f,           0.f, 0.f},
        vec4f{          0.f,           0.f,           1.f, 0.f},
        vec4f{translation.x, translation.y, translation.z, 1.f},
    };
}

mat4f rotationMatrix(vec3f axis, float angle) {
    const float sinAngle = std::sin(angle);
    const float cosAngle = std::cos(angle);
    const float oneMinusCos = 1.f - cosAngle;

    const float xx = axis.x * axis.x;
    const float xy = axis.x * axis.y;
    const float xz = axis.x * axis.z;
    const float yy = axis.y * axis.y;
    const float yz = axis.y * axis.z;
    const float zz = axis.z * axis.z;

    return mat4f{
        vec4f{
              cosAngle + xx * oneMinusCos,
              xy * oneMinusCos + axis.z * sinAngle,
              xz * oneMinusCos - axis.y * sinAngle,
              0.f                                                    },
        vec4f{
              xy * oneMinusCos - axis.z * sinAngle,
              cosAngle + yy * oneMinusCos,
              yz * oneMinusCos + axis.x * sinAngle,
              0.f                                                    },
        vec4f{
              xz * oneMinusCos + axis.y * sinAngle,
              yz * oneMinusCos - axis.x * sinAngle,
              cosAngle + zz * oneMinusCos,
              0.f                                                    },
        vec4f{                                 0.f,     0.f, 0.f, 1.f},
    };
}

vec2f sphericalCoordinates(vec3f cartesian) {
    const float phi = std::acos(cartesian.y);
    // TODO: make sure camera is using an rhs csys
    float theta = std::atan2(-cartesian.z, cartesian.x);
    theta = (theta < 0.f) ? theta + cobalt::kPI : theta;
    const float u = ((theta) / (2.f * cobalt::kPI));
    const float v = phi / cobalt::kPI;
    return {u, v};
}

} // namespace cobalt::utils
