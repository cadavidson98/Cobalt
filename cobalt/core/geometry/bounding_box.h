#ifndef CBLT_GEOM_BOUNDING_BOX_H
#define CBLT_GEOM_BOUNDING_BOX_H

#include "math/simd/simd_vec4.h"
#include "ray.h"

#include <limits>

namespace cblt::geom {

struct CoAxisAlignedBoundingBox {
        simd::vec3f min;
        simd::vec3f max;

        static simd::vec3f Center(const CoAxisAlignedBoundingBox &aabb) {
            return 0.5f * (aabb.min + aabb.max);
        };

        static simd::vec3f Scales(const CoAxisAlignedBoundingBox &aabb) {
            return aabb.max - aabb.min;
        }

        static CoAxisAlignedBoundingBox
        Union(const CoAxisAlignedBoundingBox &lhs, const CoAxisAlignedBoundingBox &rhs) {
            return {
                .min = simd::min(lhs.min, rhs.min),
                .max = simd::max(lhs.max, rhs.max),
            };
        }
};
} // namespace cblt::geom

#endif // CBLT_GEOM_BOUNDING_BOX_H
