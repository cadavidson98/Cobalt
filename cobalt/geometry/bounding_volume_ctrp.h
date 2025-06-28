#ifndef CBLT_GEOM_BOUNDING_VOLUME_H
#define CBLT_GEOM_BOUNDING_VOLUME_H

#include "bounding_box.h"
#include "bounding_volume.h"
#include "intersection.h"
#include "ray.h"

#include "core/callback.h"
#include "core/morton_encoding.h"
#include "core/size_types.h"
#include "math/vec3.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <span>
#include <type_traits>
#include <vector>

namespace cblt::geom {

enum class PrimitiveType : uint32_t {
    kSphere = 0,
    kTriangle,
    kPatch,
    kMesh,
};

struct Primitive {
    PrimitiveType type;
    CoAxisAlignedBoundingBox boundingBox;
    uint32_t index;
};

struct HeapArray {
    Primitive *ptr;
    size_t count;
};

template<typename Derived>
class CoPrimitiveStorage {
public:
    // caller assumes ownership of HeapArray
    HeapArray makePrimitives() const {
        return static_cast<const Derived *>(this)->makePrimitives();
    }

    CoAxisAlignedBoundingBox bounds() const {
        return static_cast<const Derived *>(this)->bounds();
    }

    void reorder(std::span<const Primitive> primitives) {
        return static_cast<Derived *>(this)->reorder(primitives);
    }

protected:
    CoPrimitiveStorage() = default;
};

template<typename Derived>
class CoBoundingVolumeLeafNode {
public:
    bool intersects(const CoRay &ray, IntersectionEvent &event) const {
        return static_cast<const Derived *>(this)->intersects(ray, event);
    }

protected:
    CoBoundingVolumeLeafNode() = default;
};

template<typename BoundingVolumeStorage>
concept isPrimitiveStorage =
    requires { std::is_base_of_v<CoPrimitiveStorage<BoundingVolumeStorage>, BoundingVolumeStorage>; };

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

    struct MortonPrimitive {
        uint32_t mortonCode;
        PrimitiveType type;
        uint32_t primitiveIdx;
    };

    uint8_t primitivesPerLeaf;
    std::shared_ptr<CoPrimitiveStorage<StorageType>> storage;

    void buildIterative(std::span<const Primitive> primitives, core::CoCallback &buildCallback);

    void buildTreelet(std::span<const MortonPrimitive> mortonCluster, const uint32_t mask);
};

template<typename StorageType>
    requires isPrimitiveStorage<StorageType>
CoBoundingVolume<StorageType>::CoBoundingVolume(const CreateWithPrimitivesInfo &createOptions)
    : primitivesPerLeaf{createOptions.maxPrimsInLeaf}, storage{createOptions.primitives} {
    const HeapArray primitives = storage->makePrimitives();

    std::span<const Primitive> treePrimitives(primitives.ptr, primitives.count);
    buildIterative(treePrimitives, createOptions.buildCallback);

    delete[] primitives.ptr;
}

template<typename StorageType>
    requires isPrimitiveStorage<StorageType>
CoBoundingVolume<StorageType>::~CoBoundingVolume() {
}

template<typename StorageType>
    requires isPrimitiveStorage<StorageType>
void CoBoundingVolume<StorageType>::buildIterative(std::span<const Primitive> primitives, core::CoCallback &buildCallback) {
    std::vector<MortonPrimitive> mortonCodes(primitives.size());

    buildCallback.pump("encoding primitives", 0);

    const simd::vec3f primitiveExtents = storage->bounds().Scales();
    auto encodePrimitive = [extent = primitiveExtents](const Primitive &primitive) {
        static constexpr float kMaxUInt32 = float(std::numeric_limits<uint32_t>::max());
        const simd::vec3f normalizedPosition = (primitive.boundingBox.Center() / extent) * kMaxUInt32;
        const std::array<float, 4> values = normalizedPosition.Values();
        return MortonPrimitive{
            .mortonCode = core::mortonEncode(values[0], values[1], values[2]),
            .type = primitive.type,
            .primitiveIdx = primitive.index,
        };
    };

    std::transform(primitives.begin(), primitives.end(), mortonCodes.begin(), encodePrimitive);

    buildCallback.pump("sorting primitives", 10);

    auto mortonComparator = [](const MortonPrimitive &lhs, const MortonPrimitive &rhs) {
        return lhs.mortonCode < rhs.mortonCode;
    };

    std::sort(mortonCodes.begin(), mortonCodes.end(), mortonComparator);

    storage->reorder();

    buildCallback.pump("finding clusters", 20);

    struct Cluster {
        size_t startIdx;
        size_t primitiveCount;
    };

    std::vector<Cluster> clusters;

    size_t startIdx = 0;
    size_t endIdx = 1;
    for (; endIdx < mortonCodes.size(); ++endIdx) {
        static constexpr uint32_t kMask = 0b00111111111111000000000000000000;
        if (mortonCodes[startIdx].mortonCode & kMask != mortonCodes[endIdx].mortonCode & kMask) {
            // note: exclude primitive at endIdx because it isn't in the same cluster
            clusters.push_back({
                .startIdx = startIdx,
                .primitiveCount = endIdx - startIdx,
            });
            startIdx = endIdx;
        }
    }

    clusters.push_back({
        .startIdx = startIdx,
        .primitiveCount = endIdx - startIdx,
    });

    std::vector<uint32_t> offsets(clusters.size());
    uint32_t total = 0;

    for (uint32_t idx = 0; idx < clusters.size(); ++idx) {
        total += clusters[idx].primitiveCount;
        offsets[idx] = total;
    }

    std::vector<auto> leafNodes

    for (const Cluster &primitiveCluster : clusters) {
        std::span<const MortonPrimitive> mortonCluster(&mortonCodes[primitiveCluster.startIdx], primitiveCluster.primitiveCount);
        buildTreelet(mortonCluster, 1 << 17);
    }
}

template<typename StorageType>
    requires isPrimitiveStorage<StorageType>
void CoBoundingVolume<StorageType>::buildTreelet(std::span<const MortonPrimitive> mortonCluster, const uint32_t mask) {
    if (mortonCluster.size() < primitivesPerLeaf || !mask) {
        const auto leaf = storage->makeLeaf();
        return;
    }

    const uint32_t frontMask = mortonCluster.front().mortonCode & mask;
    const uint32_t backMask = mortonCluster.back().mortonCode & mask;

    if (frontMask == backMask) {
        return buildTreelet(mortonCluster, mask >> 1);
    }

    auto isInterval = [mask, frontMask](uint32_t ref, const uint32_t val) {
        return ref < uint64_t((mask & val) != frontMask);
    };

    auto spliterator = std::upper_bound(mortonCluster.begin(), mortonCluster.end(), 0ul, isInterval);
    assert(spliterator != mortonCluster.end() && "should always partition sorted interval");

    buildTreelet({mortonCluster.begin(), spliterator}, mask >> 1);
    buildTreelet({spliterator, mortonCluster.end()}, mask >> 1);
}


} // namespace cblt::geom

#endif // CBLT_GEOM_BOUNDING_VOLUME_H
