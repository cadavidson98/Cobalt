#ifndef CBLT_GEOM_SPHERE_H
#define CBLT_GEOM_SPHERE_H

#include "math/simd/simd_vec3.h"

namespace cblt::geom {

struct CoSphere {
    simd::vec3f center;
    float radius;
};

} // namespace cblt::geom

#endif // CBLT_GEOM_SPHERE_H
