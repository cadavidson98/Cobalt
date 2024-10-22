#ifndef CBLT_MATERIAL_SURFACE_HELPERS_H
#define CBLT_MATERIAL_SURFACE_HELPERS_H

#include "constants.h"
#include "math_utils.h"
#include "vec3.h"

namespace cblt::material {

void calculateAnisotropyWeights(float alpha, float anisotropyStrength, float &anisoX, float &anisoY) {
    const float aspect = std::sqrt(1.f - 0.9f * anisotropyStrength);
    anisoX = std::max(.0001f, alpha / aspect);
    anisoY = std::max(.0001f, alpha * aspect);
}

} // namespace cblt::material

#endif // CBLT_MATERIAL_SURFACE_HELPERS_H
