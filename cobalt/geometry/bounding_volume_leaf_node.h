#ifndef CBLT_GEOM_BOUNDING_VOLUME_LEAF_NODE_H
#define CBLT_GEOM_BOUNDING_VOLUME_LEAF_NODE_H

#include "bounding_volume_types.h"
#include "intersection.h"

#include <memory>
#include <span>

namespace cblt::geom::crtp {

template<typename StorageType>
    requires isPrimitiveStorage<StorageType>
class CoBoundingVolumeLeafNode {
public:
    CoBoundingVolumeLeafNode(std::shared_ptr<StorageType> _storage, std::span<const MortonPrimitive> primitives) {

        static_assert(
            primitive_types<StorageType>::value != PrimitiveType::kNone,
            "storage must define primitive types"
        );

        for (const MortonPrimitive &mortonPrimitive : primitives) {
            const Primitive &primitive = mortonPrimitive.primitive;

            assert(primitive.type != PrimitiveType::kNone);
            boxIndices.push_back(primitive.index);
        }
    }

    bool intersects(const CoRay &ray, IntersectionEvent &event) const {
        constexpr PrimitiveTypes types = primitive_types<StorageType>::value;

        if constexpr (types & PrimitiveType::kSphere) {}

        if constexpr (types & PrimitiveType::kTriangle) {}

        if constexpr (types & PrimitiveType::kPatch) {}

        if constexpr (types & PrimitiveType::kBox) {}

        if constexpr (types & PrimitiveType::kMesh) {}

        return true;
    }

    CoBoundingVolumeLeafNode() = default;

private:
public:
    std::vector<uint32_t> boxIndices;
};

} // namespace cblt::geom::crtp

#endif // CBLT_GEOM_BOUNDING_VOLUME_LEAF_NODE_H
