#ifndef CBLT_RENDER_GGX_FUNCTION_H
#define CBLT_RENDER_GGX_FUNCTION_H

#include "math/constants.h"
#include "math/math_utilities.h"

namespace cblt::render {

inline float GGX(float cosTheta, float alpha) {
    const float cosSqr = cosTheta * cosTheta;
    const float denom = cosSqr * (alpha - 1.f) + 1.f;
    return alpha / (kPI * denom * denom);
}

inline float GGX_aniso(float alphaX, float alphaY, float cosPhi, float sinPhi, float cosTheta) {
    const float A = cblt::utils::sqr(cosPhi / alphaX) + cblt::utils::sqr(sinPhi / alphaY);
    float denom = kPI * alphaX * alphaY * cblt::utils::sqr(cblt::utils::sqr(cosTheta) + A);
    return 1.f / denom;
}

} // namespace cblt::render

#endif // CBLT_RENDER_GGX_FUNTION_H
