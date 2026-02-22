#ifndef CBLT_GEOM_TRIANGLE_H
#define CBLT_GEOM_TRIANGLE_H

#include "math/math_types.h"

namespace cblt::geom {

struct Triangle {
    simd::vec3f position1;
    simd::vec3f position2;
    simd::vec3f position3;
};

} // namespace cblt::geom

#endif // CBLT_GEOM_TRIANGLE_H
