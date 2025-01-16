#include "interpolation.h"

namespace cblt::geom {

CoSurface interpolatePatch(
    const simd::vec3f &position1,
    const simd::vec3f &position2,
    const simd::vec3f &position3,
    const simd::vec3f &position4,
    const vec2f localCoordinates) {
    const simd::vec3f V = simd::lerp(position2 - position1, position3 - position4, localCoordinates.y);
    const simd::vec3f U = simd::lerp(position4 - position1, position3 - position2, localCoordinates.x);
    return CoSurface{
        .tangentU = U,
        .tangentV = V,
    };
}

}  // namespace cblt::geom