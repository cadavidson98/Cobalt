#ifndef CBLT_RENDER_SURFACE_HELPERS_H
#define CBLT_RENDER_SURFACE_HELPERS_H

#include "math/constants.h"
#include "math/math_types.h"
#include "math/math_utils.h"

namespace cblt::render {

void calculateAnisotropyWeights(float alpha, float anisotropyStrength, float &anisoX, float &anisoY) {
    const float aspect = std::sqrt(1.f - 0.9f * anisotropyStrength);
    anisoX = std::max(.0001f, alpha / aspect);
    anisoY = std::max(.0001f, alpha * aspect);
}

} // namespace cblt::render

#endif // CBLT_RENDER_SURFACE_HELPERS_H
