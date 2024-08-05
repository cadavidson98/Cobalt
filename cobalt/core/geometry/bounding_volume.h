#ifndef CBLT_GEOM_BOUNDING_VOLUME_H
#define CBLT_GEOM_BOUNDING_VOLUME_H

#include "bounding_box.h"
#include "ray.h"

#include <cstdint>
#include <vector>

namespace cblt::geom {

class CoBoundingVolume {
    public:
        enum class PartitionMethod {
            Binary,
            Midpoint,
            SurfaceAreaHeuristic,
        };

        struct CreateWithBoundingBoxesInfo {
                std::vector<CoAxisAlignedBoundingBox> boxes;
                uint8_t maxPrimsInLeaf = kMaxPrimitivesPerLeaf;
                PartitionMethod partitionMethod;
        };

        CoBoundingVolume(const CreateWithBoundingBoxesInfo &createOptions);

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
        std::vector<CoAxisAlignedBoundingBox> primitives;
        std::vector<BoundingVolumeNode> boundingVolumeTree;

        void BuildBoundingVolumeTree(size_t startIdx, size_t endIdx);
};

} // namespace cblt::geom

#endif // CBLT_GEOM_BOUNDING_VOLUME_H
