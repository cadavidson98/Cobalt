#ifndef CBLT_GEOM_QUAD_H
#define CBLT_GEOM_QUAD_H

#include "simd/simd_vec4.h"

namespace cblt::geom {
struct CoQuad {
    simd::vec3f position1;
    simd::vec3f position2;
    simd::vec3f position3;
    simd::vec3f position4;
};
}; // namespace cblt::geom

#endif // CBLT_GEOM_QUAD_H
