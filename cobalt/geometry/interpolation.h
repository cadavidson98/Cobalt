#ifndef CBLT_GEOM_INTERPOLATION_H
#define CBLT_GEOM_INTERPOLATION_H

#include "math/simd/simd_vec3.h"
#include "math/vec2.h"

namespace cblt::geom {

struct CoSurface {
    simd::vec3f tangentU;
    simd::vec3f tangentV;
};

CoSurface interpolatePatch(
    const simd::vec3f &position1,
    const simd::vec3f &position2,
    const simd::vec3f &position3,
    const simd::vec3f &position4,
    const vec2f localCoordinates
);

} // namespace cblt::geom

#endif // CBLT_GEOM_INTERPOLATION_H
