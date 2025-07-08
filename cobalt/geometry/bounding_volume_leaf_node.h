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

    CoBoundingVolumeLeafNode(std::shared_ptr<StorageType> storage, std::span<const MortonPrimitive> primitives)
        : storage{storage} {

        static_assert(primitive_types<StorageType>::value != PrimitiveType::kNone, "storage must define primitive types");

        for (const MortonPrimitive &mortonPrimitive : primitives) {
            const Primitive &primitive = mortonPrimitive.primitive;
        
            assert(primitive.type != PrimitiveType::kNone);
            if (primitive.type == kSphere) {
                spheres.push_back(primitive.index);
            } else if (primitive.type == PrimitiveType::kTriangle) {
                triangles.push_back(primitive.index);
            } else if (primitive.type == PrimitiveType::kPatch) {
                patches.push_back(primitive.index);
            } else if (primitive.type == PrimitiveType::kBox) {
                boxes.push_back(primitive.index);
            } else if (primitive.type == PrimitiveType::kMesh) {
                meshes.push_back(primitive.index);
            }
        }

    }

    bool intersects(const CoRay &ray, IntersectionEvent &event) const {
        constexpr PrimitiveTypes types = primitive_types<StorageType>::value;

        if constexpr (types & PrimitiveType::kSphere) {

        }

        if constexpr (types & PrimitiveType::kTriangle) {

        }

        if constexpr (types & PrimitiveType::kPatch) {

        }

        if constexpr (types & PrimitiveType::kBox) {

        }

        if constexpr (types & PrimitiveType::kMesh) {

        }

        return true;
    }

    CoBoundingVolumeLeafNode() = default;
private:

    using PrimitiveIndices = std::vector<uint32_t>;

    std::shared_ptr<StorageType> storage;

    PrimitiveIndices spheres;
    PrimitiveIndices triangles;
    PrimitiveIndices patches;
    PrimitiveIndices boxes;
    PrimitiveIndices meshes;
};

}  // cblt::geom::crtp

#endif  // CBLT_GEOM_BOUNDING_VOLUME_LEAF_NODE_H