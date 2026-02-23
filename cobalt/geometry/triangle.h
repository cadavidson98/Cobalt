#ifndef COBALT_GEOM_TRIANGLE_H
#define COBALT_GEOM_TRIANGLE_H

#include "math/math_types.h"

namespace cobalt::geom {

struct Triangle {
    simd::vec3f position1;
    simd::vec3f position2;
    simd::vec3f position3;
};

} // namespace cobalt::geom

#endif // COBALT_GEOM_TRIANGLE_H
