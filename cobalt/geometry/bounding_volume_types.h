#ifndef CBLT_GEOM_BOUNDING_VOLUME_TYPES_H
#define CBLT_GEOM_BOUNDING_VOLUME_TYPES_H

#include "bounding_box.h"

#include "core/size_types.h"

#include <limits>
#include <span>
#include <vector>

namespace cblt::geom {

enum PrimitiveType : uint32_t {
    kNone = 0,
    kTriangle = 1,
    kPatch = 2,
    kSphere = 4,
    kMesh = 8,
};

using PrimitiveTypes = uint32_t;

struct Primitive {
    PrimitiveType type;
    uint32_t index;
};

struct MortonPrimitive {
    uint32_t mortonCode;
    Primitive primitive;
    AxisAlignedBoundingBox boundingBox;
};

struct PrimitiveExtent {
    uint32_t start = uint32_t(-1);
    uint32_t count = 0;
};

struct IntersectionResult {
    float hitTime = std::numeric_limits<float>::max();
    Primitive primitive;
    Primitive geometry;

    explicit operator bool() const {
        return hitTime != std::numeric_limits<float>::max();
    }
};

template<class T>
struct storageExtent {
    using value = void;
};

template<class StorageType>
concept isStorage = requires(
    StorageType storage,
    const storageExtent<StorageType>::value &extent,
    const Ray &ray,
    std::span<MortonPrimitive> mortonPrimitives
) {
    { !std::is_void_v<storageExtent<StorageType>> };
    { storage.mortonEncodePrimitives() } -> std::same_as<std::vector<MortonPrimitive>>;
    { storage.reorder(mortonPrimitives) } -> std::same_as<void>;
    { storage.intersects(extent, ray) } -> std::same_as<IntersectionResult>;
};

} // namespace cblt::geom

#endif // CBLT_GEOM_BOUNDING_VOLUME_TYPES_H
