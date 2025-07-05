#ifndef CBLT_GEOM_BOUNDING_VOLUME_CRTP_INL
#define CBLT_GEOM_BOUNDING_VOLUME_CRTP_INL

#include "bounding_volume_crtp.h"

#include "core/algorithms.h"

namespace cblt::geom::crtp {

    template<typename StorageType>
    requires isPrimitiveStorage<StorageType>
CoBoundingVolume<StorageType>::CoBoundingVolume(const CreateWithPrimitivesInfo &createOptions)
    : primitivesPerLeaf{createOptions.maxPrimsInLeaf}, storage{createOptions.primitives} {

    buildIterative(createOptions.buildCallback);
}

template<typename StorageType>
    requires isPrimitiveStorage<StorageType>
CoBoundingVolume<StorageType>::~CoBoundingVolume() {
}

template<typename StorageType>
    requires isPrimitiveStorage<StorageType>
void CoBoundingVolume<StorageType>::buildIterative(const core::CoCallback &buildCallback) {
    // buildCallback.pump("encoding primitives", 0);

    std::vector<MortonPrimitive> mortonEncodedPrimitives = storage->mortonEncodePrimitives();

    // buildCallback.pump("sorting primitives", 10);

    auto mortonKeyer = [](const MortonPrimitive &lhs) -> uint32_t {
        return lhs.mortonCode;
    };

    core::radix_sort<30>(mortonEncodedPrimitives.begin(), mortonEncodedPrimitives.end(), mortonKeyer);

    storage->reorder(mortonEncodedPrimitives);

    // buildCallback.pump("finding clusters", 20);

    buildLeafNodes(mortonEncodedPrimitives);

    // buildCallback.pump("building interior nodes");
}

template<typename StorageType>
    requires isPrimitiveStorage<StorageType>
void CoBoundingVolume<StorageType>::buildLeafNodes(
    std::span<const MortonPrimitive> primitives
) {
    std::vector<Cluster> clusters;
    
    size_t startIdx = 0;
    size_t endIdx = 1;
    for (; endIdx < primitives.size(); ++endIdx) {
        static constexpr uint32_t kMask = 0b00111111110000000000000000000000;
        const uint32_t startMask = primitives[startIdx].mortonCode & kMask;
        const uint32_t endMask = primitives[endIdx].mortonCode & kMask;
        if (startMask != endMask) {
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
    
    auto computeMaxNodesInTreelet = [](const Cluster &cluster) {
        return cluster.primitiveCount * 2 - 1;
    };
    
    uint32_t total = 0;
    for (uint32_t idx = 0; idx < clusters.size(); ++idx) {
        offsets[idx] = total;
        total += computeMaxNodesInTreelet(clusters[idx]);
    }
    
    leafNodes.resize(total);

    for (uint32_t idx = 0; idx < clusters.size(); ++idx) {
        const Cluster &cluster = clusters[idx];
        std::span<const MortonPrimitive> primitiveCluster =
            primitives.subspan(cluster.startIdx, cluster.primitiveCount);
        uint32_t currentLeafNodeIdx = 0;
        const uint32_t startIdx = idx > 0 ? offsets[idx - 1] : 0;
        const uint32_t endIdx = offsets[idx];
        std::span<LeafNodeType> clusterLeafNodes(
            std::next(leafNodes.begin(), startIdx),
            std::next(leafNodes.begin(), endIdx)
        );
        buildTreelet(primitiveCluster, leafNodes, currentLeafNodeIdx, 1 << 22);
        assert(currentLeafNodeIdx <= computeMaxNodesInTreelet(cluster));
    }
}

template<typename StorageType>
    requires isPrimitiveStorage<StorageType>
void CoBoundingVolume<StorageType>::buildTreelet(
    std::span<const MortonPrimitive> mortonCluster,
    std::span<LeafNodeType> leafNodes,
    uint32_t &currentLeafNodeIdx,
    const uint32_t mask
) {
    if (mortonCluster.size() < primitivesPerLeaf || !mask) {
        leafNodes[currentLeafNodeIdx++] = LeafNodeType(storage, mortonCluster);
        return;
    }

    const uint32_t frontMask = mortonCluster.front().mortonCode & mask;
    const uint32_t backMask = mortonCluster.back().mortonCode & mask;

    if (frontMask == backMask) {
        return buildTreelet(mortonCluster, leafNodes, currentLeafNodeIdx, mask >> 1);
    }

    auto isInterval = [mask, frontMask](uint32_t ref, const MortonPrimitive &val) {
        return ref < uint32_t((mask & val.mortonCode) != frontMask);
    };

    auto spliterator = std::upper_bound(mortonCluster.begin(), mortonCluster.end(), 0u, isInterval);
    assert(spliterator != mortonCluster.end() && "should always partition sorted interval");

    buildTreelet({mortonCluster.begin(), spliterator}, leafNodes, currentLeafNodeIdx, mask >> 1);
    buildTreelet({spliterator, mortonCluster.end()}, leafNodes, currentLeafNodeIdx,mask >> 1);
}

}  // namespace cblt::geom::crtp

#endif  // CBLT_GEOM_BOUNDING_VOLUME_CRTP_INL