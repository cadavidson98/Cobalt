#ifndef CBLT_GEOM_BOUNDING_VOLUME_TYPES_H
#define CBLT_GEOM_BOUNDING_VOLUME_TYPES_H

#include "bounding_box.h"

#include "core/size_types.h"

#include <span>
#include <vector>

namespace cblt::geom::crtp {

enum PrimitiveType : uint32_t {
    kNone = 0,
    kSphere = 1,
    kTriangle = 2,
    kPatch = 4,
    kBox = 8,
    kMesh = 16,
};

using PrimitiveTypes = uint32_t;

struct Primitive {
    PrimitiveType type;
    CoAxisAlignedBoundingBox boundingBox;
    uint32_t index;
};

struct MortonPrimitive {
    uint32_t mortonCode;
    Primitive primitive;
};

template<class Derived>
struct CoPrimitiveStorageBase {
public:
    std::vector<MortonPrimitive> mortonEncodePrimitives() const {
        return static_cast<const Derived *>(this)->mortonEncodePrimitives();
    }

    geom::CoAxisAlignedBoundingBox bounds() const {
        return static_cast<const Derived *>(this)->bounds();
    }

    void reorder(std::span<MortonPrimitive> primitives) {
        return static_cast<Derived *>(this)->reorder(primitives);
    }

protected:
    CoPrimitiveStorageBase() = default;
};

template<typename T>
struct primitive_types {
    static const PrimitiveTypes value = PrimitiveType::kNone;
};

template<typename BoundingVolumeStorage>
concept isPrimitiveStorage =
    requires { std::is_base_of_v<CoPrimitiveStorageBase<BoundingVolumeStorage>, BoundingVolumeStorage>; };

} // namespace cblt::geom::crtp

#endif // CBLT_GEOM_BOUNDING_VOLUME_TYPES_H
