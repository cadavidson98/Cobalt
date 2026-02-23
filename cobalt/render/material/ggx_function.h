#ifndef COBALT_RENDER_GGX_FUNCTION_H
#define COBALT_RENDER_GGX_FUNCTION_H

#include "math/constants.h"
#include "math/math_utilities.h"

namespace cobalt::render {

inline float GGX(float cosTheta, float alpha) {
    const float cosSqr = cosTheta * cosTheta;
    const float denom = cosSqr * (alpha - 1.f) + 1.f;
    return alpha / (kPI * denom * denom);
}

inline float GGX_aniso(float alphaX, float alphaY, float cosPhi, float sinPhi, float cosTheta) {
    const float A = cobalt::utils::sqr(cosPhi / alphaX) + cobalt::utils::sqr(sinPhi / alphaY);
    float denom = kPI * alphaX * alphaY * cobalt::utils::sqr(cobalt::utils::sqr(cosTheta) + A);
    return 1.f / denom;
}

} // namespace cobalt::render

#endif // COBALT_RENDER_GGX_FUNTION_H
