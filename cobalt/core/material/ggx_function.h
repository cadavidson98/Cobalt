#ifndef CBLT_MATERIAL_GGX_FUNCTION_H
#define CBLT_MATERIAL_GGX_FUNCTION_H

#include "math/constants.h"
#include "math/math_utils.h"

namespace cblt::material {

inline float GGX(float cosTheta, float alpha) {
    const float cosSqr = cosTheta * cosTheta;
    const float denom = cosSqr * (alpha - 1.f) + 1.f;
    return alpha / (kPI * denom * denom);
}

inline float GGX_aniso(float alphaX, float alphaY, float cosPhi, float sinPhi, float cosTheta) {
    const float A = sqr(cosPhi / alphaX) + sqr(sinPhi / alphaY);
    float denom = kPI * alphaX * alphaY * sqr(sqr(cosTheta) + A);
    return 1.f / denom;
}

} // namespace cblt::material

#endif // CBLT_MATERIAL_GGX_FUNTION_H
