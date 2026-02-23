#ifndef COBALT_GEOM_SPHERE_H
#define COBALT_GEOM_SPHERE_H

#include "math/math_types.h"

namespace cobalt::geom {

struct Sphere {
    simd::vec3f center;
    float radius;
};

} // namespace cobalt::geom

#endif // COBALT_GEOM_SPHERE_H
