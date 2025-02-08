#ifndef CBLT_GEOM_BOUNDING_VOLUME_H
#define CBLT_GEOM_BOUNDING_VOLUME_H

#include "bounding_box.h"
#include "intersection.h"
#include "ray.h"

#include "core/size_types.h"

#include <cassert>
#include <functional>
#include <limits>
#include <memory>
#include <numeric>
#include <queue>
#include <span>
#include <vector>

namespace cblt::geom {

class CoPrimitiveStorage {
public:
    CoPrimitiveStorage(size_t numPrimitives) {
        _primitiveIndices.resize(numPrimitives);
        std::iota(_primitiveIndices.begin(), _primitiveIndices.end(), 0);
    }

protected:
    std::vector<uint32_t> _primitiveIndices;
};

template<typename BoundingVolumeStorage>
class CoBoundingVolume {
public:
    enum class PartitionMethod {
        Binary,
        Midpoint,
        SurfaceAreaHeuristic,
    };

    struct CreateWithPrimitivesInfo {
        std::shared_ptr<BoundingVolumeStorage> primitives;
        uint8_t maxPrimsInLeaf = kMaxPrimitivesPerLeaf;
        PartitionMethod partitionMethod = PartitionMethod::Midpoint;
    };

    CoBoundingVolume(const CreateWithPrimitivesInfo &createOptions)
        : primitivesPerLeaf{createOptions.maxPrimsInLeaf}, partitionMethod{createOptions.partitionMethod},
          storage{createOptions.primitives} {
        BuildBoundingVolumeTree(0, storage->NumPrimitives());
    }

    ~CoBoundingVolume() {
    }

    bool IntersectClosest(const CoRay &ray, IntersectionEvent &intersectionEvent) const {

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
                    if (storage->PrimitivesIntersect(
                            ray,
                            currentNode.primitiveStartIdx,
                            primitiveEndIdx,
                            intersectionEvent
                        )) {
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

private:
    static const size_t kInvalidIndex = -1;
    static const uint8_t kMaxPrimitivesPerLeaf = 8;

    struct BoundingVolumeNode {
        CoAxisAlignedBoundingBox nodeBounds;
        // left child index is current node index + 1
        union {
            size_t rightChildIdx;
            size_t primitiveStartIdx;
        };
        uint8_t primitiveCount;
    };

    uint8_t primitivesPerLeaf;
    PartitionMethod partitionMethod;
    std::shared_ptr<BoundingVolumeStorage> storage;

    std::vector<BoundingVolumeNode> boundingVolumeTree;

    void BuildBoundingVolumeTree(size_t startIdx, size_t endIdx) {
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
        case PartitionMethod::Midpoint : {
            const float splitValue = boundingCenter[maxIndex];
            splitIdx = storage->Reorder(
                startIdx,
                endIdx,
                [splitValue, maxIndex](const CoAxisAlignedBoundingBox &primitiveBounds) {
                    const simd::vec3f aabbCenter = primitiveBounds.Center();
                    return aabbCenter[maxIndex] < splitValue;
                }
            );
            if (splitIdx != startIdx && splitIdx != endIdx) {
                break;
            }
        }
        case PartitionMethod::Binary :
        default : {
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
};

} // namespace cblt::geom

// #include "bounding_volume.inl"

#endif // CBLT_GEOM_BOUNDING_VOLUME_H
