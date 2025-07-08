#ifndef CBLT_GEOM_BOUNDING_VOLUME_CRTP_INL
#define CBLT_GEOM_BOUNDING_VOLUME_CRTP_INL

#include "bounding_volume_crtp.h"

#include "core/algorithms.h"
#include "geometry/bounding_box.h"
#include "math/math_utilities.h"

#include <algorithm>
#include <cstdint>
#include <numeric>

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

    auto mortonKeyer = [](const MortonPrimitive &lhs) {
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

    std::vector<uint32_t> leafNodeOffsets(clusters.size());
    std::vector<uint32_t> interiorNodeOffsets(clusters.size());

    auto computeMaxLeafNodes = [leafNodeSize=size_t(primitivesPerLeaf)](const Cluster &cluster) {
        return utils::divUp(cluster.primitiveCount, leafNodeSize);
    };

    auto computeMaxInteriorNodes = [](const uint32_t leafNodeCount) {
        return leafNodeCount - 1;
    };
    
    uint32_t totalLeafNodes = 0;
    uint32_t totalInteriorNodes = computeMaxInteriorNodes(clusters.size());
    for (uint32_t idx = 0; idx < clusters.size(); ++idx) {
        leafNodeOffsets[idx] = totalLeafNodes;
        interiorNodeOffsets[idx] = totalInteriorNodes;

        const uint32_t leafNodeCount = computeMaxLeafNodes(clusters[idx]);;

        totalLeafNodes += leafNodeCount;
        totalInteriorNodes += computeMaxInteriorNodes(leafNodeCount);
    }

    leafNodes.resize(totalLeafNodes);
    interiorNodes.resize(totalInteriorNodes);

    std::vector<TypedNode> treeletRoots(clusters.size());

    for (uint32_t idx = 0; idx < clusters.size(); ++idx) {
        const Cluster &cluster = clusters[idx];
        std::span<const MortonPrimitive> primitiveCluster =
            primitives.subspan(cluster.startIdx, cluster.primitiveCount);
        uint32_t currentInteriorNodeIdx = 0;
        uint32_t currentLeafNodeIdx = 0;

        const uint32_t interiorNodeStartIdx = interiorNodeOffsets[idx];
        const uint32_t interiorNodeEndIdx = idx < clusters.size() ? interiorNodeOffsets[idx + 1] : totalInteriorNodes;
        std::span<InteriorNode> clusterInteriorNodes(
            std::next(interiorNodes.begin(), interiorNodeStartIdx),
            std::next(interiorNodes.begin(), interiorNodeEndIdx)
        );

        const uint32_t leafNodeStartIdx = leafNodeOffsets[idx];
        const uint32_t leafNodeEndIdx = idx < clusters.size() ? leafNodeOffsets[idx + 1] : totalLeafNodes;
        std::span<LeafNodeType> clusterLeafNodes(
            std::next(leafNodes.begin(), leafNodeStartIdx),
            std::next(leafNodes.begin(), leafNodeEndIdx)
        );

        treeletRoots[idx] = buildTreelet(
            primitiveCluster,
            clusterInteriorNodes,
            clusterLeafNodes, 
            interiorNodeStartIdx,
            leafNodeStartIdx,
            currentInteriorNodeIdx,
            currentLeafNodeIdx,
            1 << 22
        );
    }

    // build tree for the treelet roots
    uint32_t treeNodeIdx = 0;
    buildClusterTree(
        treeletRoots,
        {interiorNodes.begin(), std::next(interiorNodes.begin(), clusters.size())},
        treeNodeIdx
    );
}

template<typename StorageType>
    requires isPrimitiveStorage<StorageType>
CoBoundingVolume<StorageType>::TypedNode CoBoundingVolume<StorageType>::buildTreelet(
    std::span<const MortonPrimitive> mortonCluster,
    std::span<InteriorNode> interiorNodes,
    std::span<LeafNodeType> leafNodes,
    const uint32_t interiorNodeOffset,
    const uint32_t leafNodeOffset,
    uint32_t &currentInteriorNodeIdx,
    uint32_t &currentLeafNodeIdx,
    const uint32_t mask
) {
    auto mergeBoundingBoxes = [](const CoAxisAlignedBoundingBox &lhs, const MortonPrimitive &rhs) {
        return CoAxisAlignedBoundingBox::Union(lhs, rhs.primitive.boundingBox);
    };

    const CoAxisAlignedBoundingBox boundingBox = std::accumulate(
        mortonCluster.begin(),
        mortonCluster.end(),
        mortonCluster.front().primitive.boundingBox,
        mergeBoundingBoxes
    );

    if (mortonCluster.size() < primitivesPerLeaf || !mask) {
        leafNodes[currentLeafNodeIdx] = LeafNodeType(storage, mortonCluster);
        return TypedNode {
            .boundingBox = boundingBox,
            .index = leafNodeOffset + (currentLeafNodeIdx++),
            .type = Type::kLeaf,
        };
    }

    const uint32_t frontMask = mortonCluster.front().mortonCode & mask;
    const uint32_t backMask = mortonCluster.back().mortonCode & mask;

    if (frontMask == backMask) {
        return buildTreelet(
            mortonCluster, 
            interiorNodes,
            leafNodes,
            interiorNodeOffset,
            leafNodeOffset,
            currentInteriorNodeIdx,
            currentLeafNodeIdx, 
            mask >> 1
        );
    }

    auto isInterval = [mask, frontMask](uint32_t ref, const MortonPrimitive &val) {
        return ref < uint32_t((mask & val.mortonCode) != frontMask);
    };

    auto spliterator = std::upper_bound(mortonCluster.begin(), mortonCluster.end(), 0u, isInterval);
    assert(spliterator != mortonCluster.end() && "should always partition sorted interval");

    const TypedNode interior{
        .boundingBox = boundingBox,
        .index = interiorNodeOffset + (currentInteriorNodeIdx++),
        .type = Type::kInterior,
    };

    interiorNodes[interior.index].left = buildTreelet(
        {mortonCluster.begin(), spliterator}, 
        interiorNodes,
        leafNodes,
        interiorNodeOffset,
        leafNodeOffset,
        currentInteriorNodeIdx,
        currentLeafNodeIdx,
        mask >> 1
    );
    
    interiorNodes[interior.index].right = buildTreelet(
        {spliterator, mortonCluster.end()},
        interiorNodes,
        leafNodes,
        interiorNodeOffset,
        leafNodeOffset,
        currentInteriorNodeIdx,
        currentLeafNodeIdx,
        mask >> 1
    );

    return interior;
}

template<typename StorageType>
    requires isPrimitiveStorage<StorageType>
CoBoundingVolume<StorageType>::TypedNode CoBoundingVolume<StorageType>::buildClusterTree(
    std::span<const TypedNode> clusters,
    std::span<InteriorNode> nodes,
    uint32_t &currentNodeIdx
) {
    if (clusters.size() == 1) {
        return clusters.front();
    } else if (clusters.empty()) {
        return TypedNode{};
    }

    auto mergeBoundingBoxes = [](const CoAxisAlignedBoundingBox &lhs, const TypedNode &rhs) {
        return CoAxisAlignedBoundingBox::Union(lhs, rhs.boundingBox);
    };

    CoAxisAlignedBoundingBox boundingBox = std::accumulate(
        clusters.begin(),
        clusters.end(),
        clusters.front().boundingBox,
        mergeBoundingBoxes
    );

    const TypedNode interior{
        .boundingBox = boundingBox,
        .index = currentNodeIdx++,
        .type = Type::kInterior,
    };

    const size_t splitIdx = clusters.size() >> 1;
    nodes[interior.index].left = buildClusterTree(
        clusters.subspan(0, splitIdx),
        nodes,
        currentNodeIdx
    );

    nodes[interior.index].right = buildClusterTree(
        clusters.subspan(splitIdx, clusters.size() - splitIdx),
        nodes,
        currentNodeIdx
    );

    return interior;
}

}  // namespace cblt::geom::crtp

#endif  // CBLT_GEOM_BOUNDING_VOLUME_CRTP_INL