#ifndef CBLT_GEOM_BOUNDING_BOX_H
#define CBLT_GEOM_BOUNDING_BOX_H

#include "math/simd/simd_vec4.h"
#include "ray.h"

#include <limits>

namespace cblt::geom {

struct CoAxisAlignedBoundingBox {
        simd::vec4f min;
        simd::vec4f max;

        static simd::vec4f Center(const CoAxisAlignedBoundingBox &aabb) {
            return 0.5f * (aabb.min + aabb.max);
        };

        static simd::vec4f Scales(const CoAxisAlignedBoundingBox &aabb) {
            return aabb.max - aabb.min;
        }

        static CoAxisAlignedBoundingBox
        Union(const CoAxisAlignedBoundingBox &lhs, const CoAxisAlignedBoundingBox &rhs) {
            return {
                .min = simd::min(lhs.min, rhs.min),
                .max = simd::max(lhs.max, rhs.max),
            };
        }

        static bool intersect(const CoRay &ray, const CoAxisAlignedBoundingBox &aabb, float &timeMin, float &timeMax) {
            const simd::vec4f minIntersectTimes = (aabb.min - ray.pos) * ray.invDir;
            const simd::vec4f maxIntersectTimes = (aabb.max - ray.pos) * ray.invDir;

            simd::vec4f closestTimes = simd::min(minIntersectTimes, maxIntersectTimes);
            simd::vec4f farthestTimes = simd::max(minIntersectTimes, maxIntersectTimes);
            // make sure the .w component doesn't screw up the reduction!
            closestTimes.w = std::numeric_limits<float>::lowest();
            farthestTimes.w = std::numeric_limits<float>::max();

            timeMin = simd::reduceMax(closestTimes);
            timeMax = simd::reduceMin(farthestTimes);

            if (timeMin < 0.f) {
                timeMin = timeMax;
            }
            return timeMax > 0.f && timeMin < ray.maxDist && timeMax >= timeMin;
        }
};
} // namespace cblt::geom

#endif // CBLT_GEOM_BOUNDING_BOX_H
