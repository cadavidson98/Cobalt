#ifndef CBLT_RENDER_SMITH_GEOMETRY_H
#define CBLT_RENDER_SMITH_GEOMETRY_H

#include "math/math_types.h"
#include "math/math_utilities.h"

#include <cmath>

namespace cblt::render {

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
    float sinCosAniso = cblt::utils::sqr(dot(omegaI, X) * alphaX) + cblt::utils::sqr(dot(omegaI, Y) * alphaY);
    float tanSqr = (1.f - cblt::utils::sqr(nDotI)) / (cblt::utils::sqr(nDotI));
    const float lambdaI = std::sqrt(1.f + sinCosAniso * tanSqr);
    // shadowing
    sinCosAniso = cblt::utils::sqr(dot(omegaO, X) * alphaX) + cblt::utils::sqr(dot(omegaO, Y) * alphaY);
    tanSqr = (1.f - cblt::utils::sqr(nDotO)) / (cblt::utils::sqr(nDotO));
    float lambdaO = std::sqrt(1.f + sinCosAniso * tanSqr);

    return 2.0f / (lambdaI + lambdaO);
}

inline float smithPartialGeom(const vec3f &omega, const vec3f &halfway, float alpha) {
    float wDotH = absDot(omega, halfway);

    float tanSqr = (1.f - wDotH * wDotH) / (wDotH * wDotH);
    return 2.f / (1.f + std::sqrt(1.f + alpha * alpha * tanSqr));
}

inline float smithPartialGeomAniso(float alphaX, float alphaY, float n_dot_v, float vDotX, float vDotY) {
    float sinCosAniso = cblt::utils::sqr(vDotX * alphaX) + cblt::utils::sqr(vDotY * alphaY);
    float tanSqr = (1.f - n_dot_v * n_dot_v) / (n_dot_v * n_dot_v);
    float denom = 1.f + std::sqrt(1.f + sinCosAniso * tanSqr);
    return 2.f / denom;
}

} // namespace cblt::render

#endif // CBLT_RENDER_SMITH_GEOMETRY_H
