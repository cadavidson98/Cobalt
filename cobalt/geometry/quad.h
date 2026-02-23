#ifndef COBALT_GEOM_QUAD_H
#define COBALT_GEOM_QUAD_H

#include "math/math_types.h"

namespace cobalt::geom {
struct Quad {
    simd::vec3f position1;
    simd::vec3f position2;
    simd::vec3f position3;
    simd::vec3f position4;
};
}; // namespace cobalt::geom

#endif // COBALT_GEOM_QUAD_H
