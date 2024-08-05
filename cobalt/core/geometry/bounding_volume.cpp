#include "bounding_volume.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <queue>

namespace cblt::geom {

CoBoundingVolume::CoBoundingVolume(const CreateWithBoundingBoxesInfo &createOptions)
    : primitivesPerLeaf{createOptions.maxPrimsInLeaf}, partitionMethod{createOptions.partitionMethod},
      primitives{std::move(createOptions.boxes)} {
    BuildBoundingVolumeTree(0, primitives.size());
}

bool CoBoundingVolume::IntersectClosest(const CoRay &ray) const {
    float timeMin, timeMax;

    std::deque<size_t> nodeStack;
    nodeStack.push_back(0);
    while (!nodeStack.empty()) {
        const size_t currentNodeIdx = nodeStack.back();
        nodeStack.pop_back();
        if (currentNodeIdx == kInvalidIndex) {
            continue;
        }
        const BoundingVolumeNode &currentNode = boundingVolumeTree[currentNodeIdx];
        if (CoAxisAlignedBoundingBox::intersect(ray, currentNode.nodeBounds, timeMin, timeMax)) {
            if (currentNode.primitiveStartIdx != kInvalidIndex) {
                // check for primitive hits
                const size_t primitiveEndIdx = currentNode.primitiveStartIdx + currentNode.primitiveCount;
                for (size_t primitiveIdx = currentNode.primitiveStartIdx; primitiveIdx < primitiveEndIdx;
                     ++primitiveIdx) {
                    if (CoAxisAlignedBoundingBox::intersect(ray, primitives[primitiveIdx], timeMin, timeMax)) {
                        return true;
                    }
                }
            }
            nodeStack.push_back(currentNode.rightChildIdx);
            nodeStack.push_back(currentNodeIdx + 1);
        }
    }
    return false;
}

void CoBoundingVolume::BuildBoundingVolumeTree(size_t startIdx, size_t endIdx) {

    if (startIdx > endIdx) {
        return;
    }

    const size_t numPrimitives = endIdx - startIdx;

    // TODO: don't use copy constructor
    static constexpr float minFloat = std::numeric_limits<float>::lowest();
    static constexpr float maxFloat = std::numeric_limits<float>::max();
    CoAxisAlignedBoundingBox regionBounds = {
        .min = simd::vec4f(maxFloat, maxFloat, maxFloat, 1.f),
        .max = simd::vec4f(minFloat, minFloat, minFloat, 1.f),
    };

    for (size_t idx = startIdx; idx < endIdx; ++idx) {
        regionBounds = CoAxisAlignedBoundingBox::Union(primitives[idx], regionBounds);
    }

    if (numPrimitives <= primitivesPerLeaf) {
        BoundingVolumeNode leafNode{
            .nodeBounds = primitives[startIdx],
            .rightChildIdx = kInvalidIndex,
            .primitiveStartIdx = startIdx,
            .primitiveCount = static_cast<uint8_t>(numPrimitives),
        };
        boundingVolumeTree.push_back(std::move(leafNode));
        return;
    }

    // split on largest axis
    const simd::vec4f boundingDimensions = CoAxisAlignedBoundingBox::Scales(regionBounds);
    const simd::vec4f boundingCenter = CoAxisAlignedBoundingBox::Center(regionBounds);
    // need to get largest axis
    const std::array<float, 4> dimsArray = boundingDimensions.asArray();
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
    case PartitionMethod::Midpoint :
    default :
        auto primitiveStart = std::next(primitives.begin(), startIdx);
        auto primitiveEnd = std::next(primitives.begin(), endIdx);
        const float splitValue = boundingCenter[maxIndex];
        const auto primitiveSplit =
            std::partition(primitiveStart, primitiveEnd, [splitValue, maxIndex](const CoAxisAlignedBoundingBox &aabb) {
                const simd::vec4f aabbCenter = CoAxisAlignedBoundingBox::Center(aabb);
                return aabbCenter[maxIndex] < splitValue;
            });
        splitIdx = std::distance(primitives.begin(), primitiveSplit);
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
