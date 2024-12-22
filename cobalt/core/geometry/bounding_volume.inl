#ifndef CBLT_GEOM_BOUNDING_VOLUME_INL
#define CBLT_GEOM_BOUNDING_VOLUME_INL
#include "bounding_volume.h"

#include "intersection.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <queue>

namespace cblt::geom {

template<typename PrimitiveType, typename Bounding, typename Intersector>
CoBoundingVolume<typename PrimitiveType, typename Bounding, typename Intersector>::CoBoundingVolume(const CreateWithPrimitivesInfo &createOptions)
    : primitivesPerLeaf{createOptions.maxPrimsInLeaf}, partitionMethod{createOptions.partitionMethod},
      primitives{createOptions.primitives} {
    BuildBoundingVolumeTree(0, primitives.size() - 1);
}

template<typename PrimitiveType, typename Bounding, typename Intersector>
CoBoundingVolume<typename PrimitiveType, typename Bounding, typename Intersector>::~CoBoundingVolume() {
}

template<typename PrimitiveType, typename Bounding, typename Intersector>
bool CoBoundingVolume<typename PrimitiveType, typename Bounding, typename Intersector>::IntersectClosest(const CoRay &ray) const {
    IntersectionEvent intersectionEvent;
    Intersector boxIntersector{ray, intersectionEvent};

    std::deque<size_t> nodeStack;
    nodeStack.push_back(0);
    while (!nodeStack.empty()) {
        const size_t currentNodeIdx = nodeStack.back();
        nodeStack.pop_back();
        if (currentNodeIdx == kInvalidIndex) {
            continue;
        }
        const BoundingVolumeNode &currentNode = boundingVolumeTree[currentNodeIdx];
        if (rayAxisAlignedBoundingBoxIntersection(ray, currentNode.nodeBounds, intersectionEvent)) {
            if (currentNode.primitiveStartIdx != kInvalidIndex) {
                // check for primitive hits
                const size_t primitiveEndIdx = currentNode.primitiveStartIdx + currentNode.primitiveCount;
                for (size_t primitiveIdx = currentNode.primitiveStartIdx; primitiveIdx < primitiveEndIdx;
                     ++primitiveIdx) {
                    if (boxIntersector(primitives[primitiveIdx])) {
                        return true;
                    }
                }
                continue;
            }
            const size_t leftChildIdx = currentNodeIdx + 1; 
            nodeStack.push_back(currentNode.rightChildIdx);
            nodeStack.push_back(leftChildIdx);
        }
    }
    return false;
}

template<typename PrimitiveType, typename Bounding, typename Intersector>
void CoBoundingVolume<typename PrimitiveType, typename Bounding, typename Intersector>::BuildBoundingVolumeTree(size_t startIdx, size_t endIdx) {

    if (startIdx > endIdx) {
        return;
    }

    const size_t numPrimitives = endIdx - startIdx + 1;

    // TODO: don't use copy constructor
    static constexpr float minFloat = std::numeric_limits<float>::lowest();
    static constexpr float maxFloat = std::numeric_limits<float>::max();
    CoAxisAlignedBoundingBox regionBounds = {
        .min = simd::vec3f(maxFloat, maxFloat, maxFloat),
        .max = simd::vec3f(minFloat, minFloat, minFloat),
    };

    Bounding primitiveBounder;
    for (size_t idx = startIdx; idx < endIdx; ++idx) {
        regionBounds = CoAxisAlignedBoundingBox::Union(primitiveBounder(primitives[idx]), regionBounds);
    }

    if (numPrimitives <= primitivesPerLeaf) {
        BoundingVolumeNode leafNode{
            .nodeBounds = primitiveBounder(primitives[startIdx]),
            .rightChildIdx = kInvalidIndex,
            .primitiveStartIdx = startIdx,
            .primitiveCount = static_cast<uint8_t>(numPrimitives),
        };
        boundingVolumeTree.push_back(std::move(leafNode));
        return;
    }

    // split on largest axis
    const simd::vec3f boundingDimensions = CoAxisAlignedBoundingBox::Scales(regionBounds);
    const simd::vec3f boundingCenter = CoAxisAlignedBoundingBox::Center(regionBounds);
    // need to get largest axis
    const std::array<float, 3> dimsArray = {boundingDimensions.x, boundingDimensions.y, boundingDimensions.z};
    size_t maxIndex = 0;
    float maxDimension = dimsArray[0];
    for (size_t idx = 1; idx < 3; ++idx) {
        if (maxDimension < dimsArray[idx]) {
            maxIndex = idx;
            maxDimension = dimsArray[idx];
        }
    }

    // my vote is for reordering w/ CSR style indexing for the leaves
    size_t splitIdx = startIdx;
    switch (partitionMethod) {
    case PartitionMethod::Midpoint: {
        auto primitiveStart = std::next(primitives.begin(), startIdx);
        auto primitiveEnd = std::next(primitives.begin(), endIdx);
        const float splitValue = boundingCenter[maxIndex];
        const auto primitiveSplit =
            std::partition(primitiveStart, primitiveEnd, [splitValue, maxIndex](const PrimitiveType &primitive) {
                static Bounding bounder;
                const CoAxisAlignedBoundingBox aabb = bounder(primitive);
                const simd::vec3f aabbCenter = CoAxisAlignedBoundingBox::Center(aabb);
                return aabbCenter[maxIndex] < splitValue;
            });
        splitIdx = std::distance(primitives.begin(), primitiveSplit);
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
        .primitiveStartIdx = kInvalidIndex,
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