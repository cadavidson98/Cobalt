#ifndef COBALT_GEOM_BOUNDING_VOLUME_H
#define COBALT_GEOM_BOUNDING_VOLUME_H

#include "bounding_box.h"
#include "bounding_volume_types.h"
#include "intersection.h"

#include "core/size_types.h"

#include <cassert>
#include <memory>
#include <span>
#include <vector>

namespace cobalt::geom {

// Linear Bounding Volume Heirarchy based on Karras:
// 'Maximizing Parallelism in the nstruction of BVHs, Octrees, and k-d Trees'
template<typename StorageType>
    requires isStorage<StorageType>
class BoundingVolume {
public:
    BoundingVolume(
        const std::shared_ptr<StorageType> primitives,
        std::span<const MortonPrimitive> mortonEncodedPrimitives
    );

    ~BoundingVolume();

    IntersectionResult intersects(const Ray &ray) const;

private:
    static constexpr uint32_t kInvalidIndex = -1;
    static constexpr uint8_t kMaxPrimitivesPerLeaf = 8;

    enum class Type {
        kInvalid = -1,
        kInterior,
        kLeaf,
    };

    struct TypedNode {
        AxisAlignedBoundingBox boundingBox;
        uint32_t index = kInvalidIndex;
        Type type = Type::kInvalid;
    };

    struct InteriorNode {
        TypedNode left;
        TypedNode right;
    };

    struct LeafNode {
        typename StorageType::Extents extents;
    };

    AxisAlignedBoundingBox boundingBox;
    std::vector<InteriorNode> interiorNodes;
    std::vector<LeafNode> leafNodes;

    uint8_t primitivesPerLeaf;
    std::shared_ptr<StorageType> storage;

    TypedNode buildTreelet(
        std::span<const MortonPrimitive> mortonEncodedPrimitives,
        std::span<InteriorNode> interiorNodes,
        std::span<LeafNode> leafNodes,
        uint32_t &currentLeafNodeIndex,
        uint32_t &currentInteriorNodeIndex,
        const uint32_t mask
    );

    TypedNode buildTree(
        std::span<const TypedNode> treeletRoots,
        std::span<InteriorNode> nodes,
        uint32_t &currentNodeIdx
    );
};

} // namespace cobalt::geom

#include "bounding_volume.inl"

#endif // COBALT_GEOM_BOUNDING_VOLUME_H
