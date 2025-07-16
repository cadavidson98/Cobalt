#ifndef CBLT_GEOM_BOUNDING_VOLUME_CRTP_H
#define CBLT_GEOM_BOUNDING_VOLUME_CRTP_H

#include "bounding_box.h"
#include "bounding_volume_leaf_node.h"
#include "bounding_volume_types.h"

#include "core/callback.h"
#include "core/size_types.h"
#include "geometry/intersection.h"
#include "math/math_types.h"

#include <cassert>
#include <memory>
#include <span>
#include <vector>

namespace cblt::geom::crtp {

// Linear Bounding Volume Heirarchy based on Karras:
// 'Maximizing Parallelism in the Construction of BVHs, Octrees, and k-d Trees'
template<typename StorageType>
    requires isPrimitiveStorage<StorageType>
class CoBoundingVolume {
public:
    struct CreateWithPrimitivesInfo {
        std::shared_ptr<StorageType> primitives;
        uint8_t maxPrimsInLeaf = kMaxPrimitivesPerLeaf;
        core::CoCallback buildCallback = {};
    };

    CoBoundingVolume(const CreateWithPrimitivesInfo &createOptions);

    ~CoBoundingVolume();

    IntersectionResult intersects(const CoRay &ray) const;

private:
    static constexpr uint32_t kInvalidIndex = -1;
    static constexpr uint8_t kMaxPrimitivesPerLeaf = 8;

    enum class Type {
        kInvalid = -1,
        kInterior,
        kLeaf,
    };

    struct TypedNode {
        CoAxisAlignedBoundingBox boundingBox;
        uint32_t index = kInvalidIndex;
        Type type = Type::kInvalid;
    };

    struct InteriorNode {
        TypedNode left;
        TypedNode right;
    };

    using LeafNodeType = CoBoundingVolumeLeafNode<StorageType>;

    CoAxisAlignedBoundingBox boundingBox;
    std::vector<InteriorNode> interiorNodes;
    std::vector<LeafNodeType> leafNodes;

    uint8_t primitivesPerLeaf;
    std::shared_ptr<StorageType> storage;

    TypedNode buildTreelet(
        std::span<const MortonPrimitive> mortonEncodedPrimitives,
        std::span<InteriorNode> interiorNodes,
        std::span<LeafNodeType> leafNodes,
        uint32_t &currentLeafNodeIndex,
        uint32_t &currentInteriorNodeIndex,
        const uint32_t mask
    );

    TypedNode
    buildTree(std::span<const TypedNode> treeletRoots, std::span<InteriorNode> nodes, uint32_t &currentNodeIdx);
};

} // namespace cblt::geom::crtp

#include "bounding_volume_crtp.inl"

#endif // CBLT_GEOM_BOUNDING_VOLUME_CRTP_H
