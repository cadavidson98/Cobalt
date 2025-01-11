#ifndef CBLT_RENDER_SMITH_GEOMETRY_H
#define CBLT_RENDER_SMITH_GEOMETRY_H

#include "math_utils.h"
#include "vec3.h"

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
    float sinCosAniso = cblt::sqr(dot(omegaI, X) * alphaX) + cblt::sqr(dot(omegaI, Y) * alphaY);
    float tanSqr = (1.f - cblt::sqr(nDotI)) / (cblt::sqr(nDotI));
    const float lambdaI = std::sqrt(1.f + sinCosAniso * tanSqr);
    // shadowing
    sinCosAniso = cblt::sqr(dot(omegaO, X) * alphaX) + cblt::sqr(dot(omegaO, Y) * alphaY);
    tanSqr = (1.f - cblt::sqr(nDotO)) / (cblt::sqr(nDotO));
    float lambdaO = std::sqrt(1.f + sinCosAniso * tanSqr);

    return 2.0f / (lambdaI + lambdaO);
}

inline float smithPartialGeom(const vec3f &omega, const vec3f &halfway, float alpha) {
    float wDotH = absDot(omega, halfway);

    float tanSqr = (1.f - wDotH * wDotH) / (wDotH * wDotH);
    return 2.f / (1.f + std::sqrt(1.f + alpha * alpha * tanSqr));
}

inline float smithPartialGeomAniso(float alphaX, float alphaY, float n_dot_v, float vDotX, float vDotY) {
    float sinCosAniso = cblt::sqr(vDotX * alphaX) + cblt::sqr(vDotY * alphaY);
    float tanSqr = (1.f - n_dot_v * n_dot_v) / (n_dot_v * n_dot_v);
    float denom = 1.f + std::sqrt(1.f + sinCosAniso * tanSqr);
    return 2.f / denom;
}

} // namespace cblt::render

#endif // CBLT_RENDER_SMITH_GEOMETRY_H
