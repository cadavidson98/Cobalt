#ifndef COBALT_RENDER_SMITH_GEOMETRY_H
#define COBALT_RENDER_SMITH_GEOMETRY_H

#include "math/math_types.h"
#include "math/math_utilities.h"

#include <cmath>

namespace cobalt::render {

inline float smithGeomAniso(
    const vec3f &omegaI,
    const vec3f &omegaO,
    const vec3f &normal,
    const vec3f &X,
    const vec3f &Y,
    const float alphaX,
    const float alphaY
) {
    const float nDotI = dot(omegaI, normal);
    const float nDotO = dot(omegaO, normal);

    // masking
    float sinCosAniso = cobalt::utils::sqr(dot(omegaI, X) * alphaX) + cobalt::utils::sqr(dot(omegaI, Y) * alphaY);
    float tanSqr = (1.f - cobalt::utils::sqr(nDotI)) / (cobalt::utils::sqr(nDotI));
    const float lambdaI = std::sqrt(1.f + sinCosAniso * tanSqr);
    // shadowing
    sinCosAniso = cobalt::utils::sqr(dot(omegaO, X) * alphaX) + cobalt::utils::sqr(dot(omegaO, Y) * alphaY);
    tanSqr = (1.f - cobalt::utils::sqr(nDotO)) / (cobalt::utils::sqr(nDotO));
    float lambdaO = std::sqrt(1.f + sinCosAniso * tanSqr);

    return 2.0f / (lambdaI + lambdaO);
}

inline float smithPartialGeom(const vec3f &omega, const vec3f &halfway, float alpha) {
    float wDotH = absDot(omega, halfway);

    float tanSqr = (1.f - wDotH * wDotH) / (wDotH * wDotH);
    return 2.f / (1.f + std::sqrt(1.f + alpha * alpha * tanSqr));
}

inline float smithPartialGeomAniso(float alphaX, float alphaY, float n_dot_v, float vDotX, float vDotY) {
    float sinCosAniso = cobalt::utils::sqr(vDotX * alphaX) + cobalt::utils::sqr(vDotY * alphaY);
    float tanSqr = (1.f - n_dot_v * n_dot_v) / (n_dot_v * n_dot_v);
    float denom = 1.f + std::sqrt(1.f + sinCosAniso * tanSqr);
    return 2.f / denom;
}

} // namespace cobalt::render

#endif // COBALT_RENDER_SMITH_GEOMETRY_H
