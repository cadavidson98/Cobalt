#ifndef CBLT_GEOM_BOUNDING_VOLUME_INL
#define CBLT_GEOM_BOUNDING_VOLUME_INL
#include "bounding_volume.h"

#include "intersection.h"

#include <cassert>
#include <functional>
#include <limits>
#include <queue>

namespace cblt::geom {

template<typename BoundingVolumeStorage>
CoBoundingVolume<typename BoundingVolumeStorage>::CoBoundingVolume(const CreateWithPrimitivesInfo &createOptions)
    : primitivesPerLeaf{createOptions.maxPrimsInLeaf}, partitionMethod{createOptions.partitionMethod},
      storage{createOptions.primitives} {
    BuildBoundingVolumeTree(0, storage->NumPrimitives());
}

template<typename BoundingVolumeStorage>
CoBoundingVolume<typename BoundingVolumeStorage>::~CoBoundingVolume() {
}

template<typename BoundingVolumeStorage>
bool CoBoundingVolume<typename BoundingVolumeStorage>::IntersectClosest(const CoRay &ray, IntersectionEvent &intersectionEvent) const {

    float closestHit = std::numeric_limits<float>::max();
    float treeMinTime = 0;
    float treeMaxTime = 0;

    std::deque<size_t> nodeStack;
    nodeStack.push_back(0);
    while (!nodeStack.empty()) {
        const size_t currentNodeIdx = nodeStack.back();
        nodeStack.pop_back();
        if (currentNodeIdx == kInvalidIndex) {
            continue;
        }
        const BoundingVolumeNode &currentNode = boundingVolumeTree[currentNodeIdx];
        if (rayAxisAlignedBoundingBoxIntersection(ray, currentNode.nodeBounds, treeMinTime, treeMaxTime) &&
            treeMinTime < closestHit) {
            if (currentNode.primitiveCount != 0) {
                // check for primitive hits
                const size_t primitiveEndIdx = currentNode.primitiveStartIdx + currentNode.primitiveCount;
                if (storage->PrimitivesIntersect(ray, currentNode.primitiveStartIdx, primitiveEndIdx, intersectionEvent)) {
                    closestHit = intersectionEvent.timeMin;
                }
                continue;
            }
            const size_t leftChildIdx = currentNodeIdx + 1; 
            nodeStack.push_back(currentNode.rightChildIdx);
            nodeStack.push_back(leftChildIdx);
        }
    }
    return closestHit < ray.maxDist;
}

template<typename BoundingVolumeStorage>
void CoBoundingVolume<typename BoundingVolumeStorage>::BuildBoundingVolumeTree(size_t startIdx, size_t endIdx) {

    if (startIdx > endIdx) {
        return;
    }

    const size_t numPrimitives = endIdx - startIdx;

    CoAxisAlignedBoundingBox regionBounds = storage->PrimitiveBounds(startIdx, endIdx);

    if (numPrimitives <= primitivesPerLeaf) {
        assert(numPrimitives + startIdx <= storage->NumPrimitives());
        BoundingVolumeNode leafNode{
            .nodeBounds = regionBounds,
            .primitiveStartIdx = startIdx,
            .primitiveCount = static_cast<uint8_t>(numPrimitives),
        };
        boundingVolumeTree.push_back(std::move(leafNode));
        return;
    }

    // split on largest axis
    const simd::vec3f boundingDimensions = regionBounds.Scales();
    const simd::vec3f boundingCenter = regionBounds.Center();
    // need to get largest axis
    const std::array<float, 4> dimensions = boundingDimensions.Values();
    size_t maxIndex = 0;
    float maxDimension = dimensions[0];
    for (size_t idx = 1; idx < 3; ++idx) {
        if (maxDimension < dimensions[idx]) {
            maxIndex = idx;
            maxDimension = dimensions[idx];
        }
    }

    size_t splitIdx = startIdx;
    switch (partitionMethod) {
    case PartitionMethod::Midpoint: {
        const float splitValue = boundingCenter[maxIndex];
        splitIdx = storage->Reorder(startIdx, endIdx, [splitValue, maxIndex](const CoAxisAlignedBoundingBox &primitiveBounds) {
                const simd::vec3f aabbCenter = primitiveBounds.Center();
                return aabbCenter[maxIndex] < splitValue;
            });
        if (splitIdx != startIdx && splitIdx != endIdx) {
            break;
        }
    }
    case PartitionMethod::Binary:
    default: {
        splitIdx = (endIdx + startIdx) >> 1;
    }
    }
    // make node
    BoundingVolumeNode node{
        .nodeBounds = regionBounds,
        .rightChildIdx = kInvalidIndex,
        .primitiveCount = 0,
    };

    boundingVolumeTree.push_back(std::move(node));
    size_t curNodeIdx = boundingVolumeTree.size();
    BuildBoundingVolumeTree(startIdx, splitIdx);
    boundingVolumeTree[curNodeIdx - 1].rightChildIdx = boundingVolumeTree.size();
    BuildBoundingVolumeTree(splitIdx, endIdx);
}

} // namespace cblt::geom

#endif   // CBLT_GEOM_BOUNDING_VOLUME_INL