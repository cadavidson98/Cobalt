#ifndef CBLT_GEOM_BOUNDING_VOLUME_LEAF_NODE_H
#define CBLT_GEOM_BOUNDING_VOLUME_LEAF_NODE_H

#include "bounding_box.h"
#include "bounding_volume_types.h"
#include "intersection.h"
#include "sphere.h"

#include <limits>
#include <span>

namespace cblt::geom::crtp {

template<typename StorageType>
    requires isPrimitiveStorage<StorageType>
class CoBoundingVolumeLeafNode {
public:
    CoBoundingVolumeLeafNode(std::span<const MortonPrimitive> primitives) {

        static_assert(
            primitive_types<StorageType>::value != PrimitiveType::kNone,
            "storage must define primitive types"
        );

        const auto expandExtent = [](const PrimitiveExtent extent, const Primitive &primitive) -> PrimitiveExtent {
            return {
                .start = std::min(extent.start, primitive.index),
                .count = extent.count + 1,
            };
        };

        for (const MortonPrimitive &mortonPrimitive : primitives) {
            const Primitive &primitive = mortonPrimitive.primitive;
            switch (primitive.type) {
            case PrimitiveType::kBox : boxes = expandExtent(boxes, primitive); continue;
            case PrimitiveType::kSphere : spheres = expandExtent(spheres, primitive); continue;
            case PrimitiveType::kTriangle : triangles = expandExtent(triangles, primitive); continue;
            case PrimitiveType::kPatch : patches = expandExtent(patches, primitive); continue;
            case PrimitiveType::kMesh : meshes = expandExtent(meshes, primitive); continue;
            default : assert(false);
            }
        }
    }

    IntersectionResult intersects(const StorageType &storage, const CoRay &ray) const {
        constexpr PrimitiveTypes types = primitive_types<StorageType>::value;

        IntersectionResult result;

        if constexpr (types & PrimitiveType::kSphere) {
            std::span<const CoSphere> spherePrimitives = storage.spheres(spheres.start, spheres.count);
            for (size_t idx = 0; idx < spheres.count; ++idx) {
                float localTimeMin = std::numeric_limits<float>::max();
                float localTimeMax = std::numeric_limits<float>::max();
                if (raySphereIntersection(ray, spherePrimitives[idx], localTimeMin, localTimeMax) && localTimeMin < result.hitTime) {
                    result.hitTime = localTimeMin;
                    result.primitive = {
                        .type = PrimitiveType::kSphere,
                        .index = idx + spheres.start,
                    };
                }
            }
        }

        if constexpr (types & PrimitiveType::kBox) {
            std::span<const CoAxisAlignedBoundingBox> boxPrimitives = storage.boxes(boxes.start, boxes.count);

            for (size_t idx = 0; idx < boxes.count; ++idx) {
                const CoAxisAlignedBoundingBox &box = boxPrimitives[idx];
                float localTimeMin = std::numeric_limits<float>::max();
                float localTimeMax = std::numeric_limits<float>::max();
                if(rayAxisAlignedBoundingBoxIntersection(ray, box, localTimeMin, localTimeMax) && localTimeMin < result.hitTime) {
                    result.hitTime = localTimeMin;
                    result.primitive = {
                        .type = PrimitiveType::kBox,
                        .index = idx + boxes.start,
                    };
                }
            }
        }

        if constexpr (types & PrimitiveType::kTriangle) {}

        if constexpr (types & PrimitiveType::kPatch) {}

        if constexpr (types & PrimitiveType::kMesh) {}

        return result;
    }

    CoBoundingVolumeLeafNode() = default;

private:
    struct PrimitiveExtent {
        uint32_t start = std::numeric_limits<uint32_t>::max();
        uint32_t count = 0;
    };

    PrimitiveExtent spheres;
    PrimitiveExtent triangles;
    PrimitiveExtent patches;
    PrimitiveExtent boxes;
    PrimitiveExtent meshes;
};

} // namespace cblt::geom::crtp

#endif // CBLT_GEOM_BOUNDING_VOLUME_LEAF_NODE_H
