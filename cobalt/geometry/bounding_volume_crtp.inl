#ifndef CBLT_GEOM_BOUNDING_VOLUME_CRTP_INL
#define CBLT_GEOM_BOUNDING_VOLUME_CRTP_INL

#include "bounding_volume_crtp.h"
#include "bounding_volume_types.h"

#include "core/algorithms.h"
#include "geometry/bounding_box.h"
#include "math/math_utilities.h"

#include <algorithm>
#include <cstdint>
#include <numeric>

namespace cblt::geom::crtp {

namespace {

struct Cluster {
    size_t startIdx = size_t(-1);
    size_t primitiveCount = 0;
};

struct NodeOffsets {
    uint32_t interiorNode;
    uint32_t leafNode;
};

std::vector<Cluster> findClusters(std::span<const MortonPrimitive> primitives) {
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

    if (endIdx - startIdx > 0) {
        clusters.push_back({
            .startIdx = startIdx,
            .primitiveCount = endIdx - startIdx,
        });
    }   

    return clusters;
}

std::vector<NodeOffsets> prefixSum(std::span<const Cluster> clusters, size_t primitivesPerLeaf) {
    std::vector<NodeOffsets> clusterOffsets(clusters.size() + 1);

    auto computeMaxLeafNodes = [leafNodeSize=primitivesPerLeaf](const Cluster &cluster) -> size_t {
        // return utils::divUp(cluster.primitiveCount, leafNodeSize);
        return cluster.primitiveCount;
    };

    auto computeMaxInteriorNodes = [](const size_t leafNodeCount) -> size_t {
        assert(leafNodeCount > 0 && "must have at least 1 leaf node");
        return leafNodeCount - 1;
    };

    uint32_t totalInteriorNodes = computeMaxInteriorNodes(clusters.size());
    uint32_t totalLeafNodes = 0;

    for (size_t idx = 0; idx < clusters.size(); ++idx) {
        clusterOffsets[idx].leafNode = totalLeafNodes;
        clusterOffsets[idx].interiorNode = totalInteriorNodes;

        const uint32_t leafNodeCount = computeMaxLeafNodes(clusters[idx]);

        totalLeafNodes += leafNodeCount;
        totalInteriorNodes += computeMaxInteriorNodes(leafNodeCount);
    }

    clusterOffsets.back() = NodeOffsets{
        .interiorNode = totalInteriorNodes,
        .leafNode = totalLeafNodes,
    };

    return clusterOffsets;
}

}  // anonymous namespace

template<typename StorageType>
    requires isPrimitiveStorage<StorageType>
CoBoundingVolume<StorageType>::CoBoundingVolume(const CreateWithPrimitivesInfo &createOptions)
    : primitivesPerLeaf{createOptions.maxPrimsInLeaf}, storage{createOptions.primitives} {
        std::vector<MortonPrimitive> mortonEncodedPrimitives = storage->mortonEncodePrimitives();

        auto mortonKeyer = [](const MortonPrimitive &lhs) {
            return lhs.mortonCode;
        };

        core::radix_sort<30>(mortonEncodedPrimitives.begin(), mortonEncodedPrimitives.end(), mortonKeyer);

        storage->reorder(mortonEncodedPrimitives);

        const std::vector<Cluster> clusters = findClusters(mortonEncodedPrimitives);

        const std::vector<NodeOffsets> offsets = prefixSum(clusters, primitivesPerLeaf);

        assert(offsets.size() == clusters.size() + 1 && "prefix sum array must be larger than clusters");

        const NodeOffsets &treeSize = offsets.back();

        leafNodes.resize(treeSize.leafNode);
        interiorNodes.resize(treeSize.interiorNode);

        std::vector<TypedNode> treeletRoots(clusters.size());

        for (size_t idx = 0; idx < clusters.size(); ++idx) {
            const Cluster &cluster = clusters[idx];
    
            auto first = mortonEncodedPrimitives.begin() + cluster.startIdx;
            auto last = first + cluster.primitiveCount;
            std::span<const MortonPrimitive> clusteredPrimitives{ first, last };
    
            uint32_t currentLeafNodeIdx = offsets[idx].leafNode;
            uint32_t currentInteriorNodeIdx = offsets[idx].interiorNode;
    
            assert(idx != offsets.size() && "must not index to the total node sizes in prefix sum array");
    
            treeletRoots[idx] = buildTreelet(
                clusteredPrimitives,
                interiorNodes,
                leafNodes,
                currentLeafNodeIdx,
                currentInteriorNodeIdx,
                1 << 21
            );
        }
    
        uint32_t currentNodeIdx = 0;
        buildTree(treeletRoots, interiorNodes, currentNodeIdx);
}

template<typename StorageType>
    requires isPrimitiveStorage<StorageType>
CoBoundingVolume<StorageType>::~CoBoundingVolume() {
}

template<typename StorageType>
    requires isPrimitiveStorage<StorageType>
CoBoundingVolume<StorageType>::TypedNode CoBoundingVolume<StorageType>::buildTreelet(
    std::span<const MortonPrimitive> mortonEncodedPrimitives,
    std::span<InteriorNode> interiorNodes,
    std::span<LeafNodeType> leafNodes,
    uint32_t &currentLeafNodeIdx,
    uint32_t &currentInteriorNodeIdx,
    const uint32_t mask
) {
    auto mergeBoundingBoxes = [](const CoAxisAlignedBoundingBox &lhs, const MortonPrimitive &rhs) {
        return CoAxisAlignedBoundingBox::Union(lhs, rhs.primitive.boundingBox);
    };

    assert(mortonEncodedPrimitives.size() > 0);

    const CoAxisAlignedBoundingBox boundingBox = std::accumulate(
        mortonEncodedPrimitives.begin(),
        mortonEncodedPrimitives.end(),
        mortonEncodedPrimitives.front().primitive.boundingBox,
        mergeBoundingBoxes
    );

    if ((mortonEncodedPrimitives.size() <= primitivesPerLeaf) || !mask) {
        const uint32_t leafNodeIdx = currentLeafNodeIdx++;

        assert(mortonEncodedPrimitives.size() <= primitivesPerLeaf);
        assert(leafNodeIdx < leafNodes.size() && "leaf node index out of bounds");

        leafNodes[leafNodeIdx] = LeafNodeType(mortonEncodedPrimitives);
        return TypedNode {
            .boundingBox = boundingBox,
            .index = leafNodeIdx,
            .type = Type::kLeaf,
        };
    }

    const uint32_t frontMask = mortonEncodedPrimitives.front().mortonCode & mask;
    const uint32_t backMask = mortonEncodedPrimitives.back().mortonCode & mask;

    if (frontMask == backMask) {
        return buildTreelet(
            mortonEncodedPrimitives, 
            interiorNodes,
            leafNodes,
            currentLeafNodeIdx,
            currentInteriorNodeIdx,
            mask >> 1
        );

        assert(false && "should never execute");
    }

    auto isInterval = [mask, frontMask](uint32_t ref, const MortonPrimitive &val) {
        return ref < uint32_t((mask & val.mortonCode) != frontMask);
    };

    auto spliterator = std::upper_bound(mortonEncodedPrimitives.begin(), mortonEncodedPrimitives.end(), 0u, isInterval);

    assert(spliterator != mortonEncodedPrimitives.begin() && spliterator != mortonEncodedPrimitives.end() && "should always partition sorted interval");

    const uint32_t interiorNodeIndex = currentInteriorNodeIdx++;

    const TypedNode interior{
        .boundingBox = boundingBox,
        .index = interiorNodeIndex,
        .type = Type::kInterior,
    };

    assert(interiorNodeIndex < interiorNodes.size() && "interior node index out of bounds");

    interiorNodes[interiorNodeIndex].left = buildTreelet(
        {mortonEncodedPrimitives.begin(), spliterator}, 
        interiorNodes,
        leafNodes,
        currentLeafNodeIdx,
        currentInteriorNodeIdx,
        mask >> 1
    );

    interiorNodes[interiorNodeIndex].right = buildTreelet(
        {spliterator, mortonEncodedPrimitives.end()},
        interiorNodes,
        leafNodes,
        currentLeafNodeIdx,
        currentInteriorNodeIdx,
        mask >> 1
    );

    return interior;
}

template<typename StorageType>
    requires isPrimitiveStorage<StorageType>
CoBoundingVolume<StorageType>::TypedNode CoBoundingVolume<StorageType>::buildTree(
    std::span<const TypedNode> treeletRoots,
    std::span<InteriorNode> nodes,
    uint32_t &currentNodeIdx
) {
    if (treeletRoots.size() == 1) {
        return treeletRoots.front();
    } else if (treeletRoots.empty()) {
        return TypedNode{};
    }

    auto mergeBoundingBoxes = [](const CoAxisAlignedBoundingBox &lhs, const TypedNode &rhs) {
        return CoAxisAlignedBoundingBox::Union(lhs, rhs.boundingBox);
    };

    CoAxisAlignedBoundingBox boundingBox = std::accumulate(
        treeletRoots.begin(),
        treeletRoots.end(),
        treeletRoots.front().boundingBox,
        mergeBoundingBoxes
    );

    const TypedNode interior{
        .boundingBox = boundingBox,
        .index = currentNodeIdx++,
        .type = Type::kInterior,
    };

    const size_t splitIdx = treeletRoots.size() >> 1;
    nodes[interior.index].left = buildTree(
        treeletRoots.subspan(0, splitIdx),
        nodes,
        currentNodeIdx
    );

    nodes[interior.index].right = buildTree(
        treeletRoots.subspan(splitIdx, treeletRoots.size() - splitIdx),
        nodes,
        currentNodeIdx
    );

    return interior;
}

}  // namespace cblt::geom::crtp

#endif  // CBLT_GEOM_BOUNDING_VOLUME_CRTP_INL