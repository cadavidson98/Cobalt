#ifndef CBLT_GEOM_BOUNDING_VOLUME_H
#define CBLT_GEOM_BOUNDING_VOLUME_H

#include "bounding_box.h"
#include "intersection.h"
#include "ray.h"

#include <cstdint>
#include <span>
#include <vector>

namespace cblt::geom {

struct boundingBoxIntersector {
        const CoRay ray;
        IntersectionEvent event;
        bool operator()(const CoAxisAlignedBoundingBox &aabb) {
            return rayAxisAlignedBoundingBoxIntersection(ray, aabb, event);
        }
};

template<typename PrimitiveType, typename Bounding, typename Intersector>
class CoBoundingVolume {
    public:
        enum class PartitionMethod {
            Binary,
            Midpoint,
            SurfaceAreaHeuristic,
        };

        struct CreateWithPrimitivesInfo {
                std::span<PrimitiveType> primitives;
                uint8_t maxPrimsInLeaf = kMaxPrimitivesPerLeaf;
                PartitionMethod partitionMethod = PartitionMethod::Midpoint;
        };

        CoBoundingVolume(const CreateWithPrimitivesInfo &createOptions);
        ~CoBoundingVolume();

        bool IntersectClosest(const CoRay &ray) const;

    private:
        static const size_t kInvalidIndex = -1;
        static const uint8_t kMaxPrimitivesPerLeaf = 8;

        struct BoundingVolumeNode {
                CoAxisAlignedBoundingBox nodeBounds;
                // left child index is current node index + 1
                size_t rightChildIdx;
                size_t primitiveStartIdx;
                uint8_t primitiveCount;
        };

        uint8_t primitivesPerLeaf;
        PartitionMethod partitionMethod;
        std::span<PrimitiveType> primitives;
        std::vector<BoundingVolumeNode> boundingVolumeTree;

        void BuildBoundingVolumeTree(size_t startIdx, size_t endIdx);
};

} // namespace cblt::geom

#include "bounding_volume.inl"

#endif // CBLT_GEOM_BOUNDING_VOLUME_H
