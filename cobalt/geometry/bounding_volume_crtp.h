#ifndef CBLT_GEOM_BOUNDING_VOLUME_CRTP_H
#define CBLT_GEOM_BOUNDING_VOLUME_CRTP_H

#include "bounding_box.h"
#include "intersection.h"

#include "core/callback.h"
#include "core/morton_encoding.h"
#include "core/size_types.h"
#include "math/math_types.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <span>
#include <type_traits>
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
        const CoAxisAlignedBoundingBox primitiveBounds = bounds();
        const simd::vec3f boundsExtent = primitiveBounds.Scales();
        const simd::vec3f boundsMin = primitiveBounds.min;
        std::vector<Primitive> storagePrimitives = primitives();
        auto encodePrimitive = [&boundsExtent, &boundsMin](const Primitive &primitive) {
            static constexpr float kFloatToUint = float((1 << 10) - 1);
            const simd::vec3f normalizedPosition = (primitive.boundingBox.Center() - boundsMin) / boundsExtent;
            const std::array<float, 4> values = (kFloatToUint * normalizedPosition).Values();
            return MortonPrimitive{
                .mortonCode = core::mortonEncode(values[0], values[1], values[2]),
                .primitive = primitive,
            };
        };
    
        std::vector<MortonPrimitive> mortonEncodedPrimitives(storagePrimitives.size());
        std::transform(
            storagePrimitives.begin(),
            storagePrimitives.end(),
            mortonEncodedPrimitives.begin(),
            encodePrimitive
        );
        
        return mortonEncodedPrimitives;
    }

    std::vector<Primitive> primitives() const {
        return static_cast<const Derived *>(this)->primitives();
    }

    geom::CoAxisAlignedBoundingBox bounds() const {
        return static_cast<const Derived *>(this)->bounds();
    }

    void reorder(std::span<const MortonPrimitive> primitives) {
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

template<typename StorageType>
    requires isPrimitiveStorage<StorageType>
class CoBoundingVolumeLeafNode {
public:

    CoBoundingVolumeLeafNode(std::shared_ptr<StorageType> storage, std::span<const MortonPrimitive> primitives)
        : storage{storage} {

        static_assert(primitive_types<StorageType>::value != PrimitiveType::kNone, "storage must define primitive types");

        CoAxisAlignedBoundingBox boundingBox;

        for (const MortonPrimitive &mortonPrimitive : primitives) {
            const Primitive &primitive = mortonPrimitive.primitive;
        
            boundingBox = CoAxisAlignedBoundingBox::Union(boundingBox, primitive.boundingBox);

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

        bounds = boundingBox;
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
    CoAxisAlignedBoundingBox bounds;

    PrimitiveIndices spheres;
    PrimitiveIndices triangles;
    PrimitiveIndices patches;
    PrimitiveIndices boxes;
    PrimitiveIndices meshes;
};

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

private:
    static const uint32_t kInvalidIndex = -1;
    static const uint8_t kMaxPrimitivesPerLeaf = 8;

    enum class Type {
        kInterior,
        kLeaf,
    };

    struct Cluster {
        size_t startIdx;
        size_t primitiveCount;
    };

    struct TypedNode {
        uint32_t index;
        Type type;
    };

    struct InteriorNode {
        TypedNode left;
        TypedNode right;
    };

    using LeafNodeType = CoBoundingVolumeLeafNode<StorageType>;

    std::vector<InteriorNode> interiorNodes;
    std::vector<LeafNodeType> leafNodes;

    uint8_t primitivesPerLeaf;
    std::shared_ptr<StorageType> storage;

    void buildIterative(const core::CoCallback &buildCallback);

    void buildLeafNodes(std::span<const MortonPrimitive> primitives);
    
    void buildTreelet(
        std::span<const MortonPrimitive> mortonCluster,
        std::span<LeafNodeType> leafNodes,
        uint32_t &currentLeafNodeIdx,
        const uint32_t mask
    );
};

} // namespace cblt::geom::crtp

#include "bounding_volume_crtp.inl"

#endif // CBLT_GEOM_BOUNDING_VOLUME_CRTP_H
