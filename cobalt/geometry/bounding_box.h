#ifndef CBLT_GEOM_BOUNDING_BOX_H
#define CBLT_GEOM_BOUNDING_BOX_H

#include "ray.h"

#include "math/math_types.h"

#include <limits>

namespace cblt::geom {

struct AxisAlignedBoundingBox {
    simd::vec3f min;
    simd::vec3f max;

    simd::vec3f Center() const {
        return 0.5f * (min + max);
    };

    simd::vec3f Scales() const {
        return max - min;
    }

    static AxisAlignedBoundingBox Union(const AxisAlignedBoundingBox &lhs, const AxisAlignedBoundingBox &rhs) {
        return {
            .min = simd::min(lhs.min, rhs.min),
            .max = simd::max(lhs.max, rhs.max),
        };
    }
};
} // namespace cblt::geom

#endif // CBLT_GEOM_BOUNDING_BOX_H
