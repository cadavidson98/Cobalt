#include "mesh.h"

#include "intersection.h"
#include "logging.h"
#include "quad.h"
#include "triangle.h"
#include "vec3.h"

#include <fstream>
#include <sstream>

CBLT_DEFINE_LOG(CoLogMesh);

namespace cblt::geom {

CoMesh::TriangleStorage::TriangleStorage(simd::vec3f *positions, vec4u *indices)
    : _positions{positions}, _indices{indices} {
}

CoMesh::TriangleStorage::~TriangleStorage() {
    delete[] _positions;
    delete[] _indices;
}

size_t CoMesh::TriangleStorage::NumPrimitives() const {
    return numIndices;
}

CoAxisAlignedBoundingBox CoMesh::TriangleStorage::PrimitiveBounds(size_t startIdx, size_t endIdx) const {
    static constexpr float minFloat = std::numeric_limits<float>::lowest();
    static constexpr float maxFloat = std::numeric_limits<float>::max();
    CoAxisAlignedBoundingBox regionBounds = {
        .min = simd::vec3f(maxFloat, maxFloat, maxFloat),
        .max = simd::vec3f(minFloat, minFloat, minFloat),
    };

    std::vector<CoAxisAlignedBoundingBox> quadBounds = _ComputePrimitiveBounds(startIdx, endIdx);

    for (size_t idx = startIdx; idx < endIdx; ++idx) {
        regionBounds.min = simd::min(regionBounds.min, quadBounds[idx].min);
        regionBounds.max = simd::max(regionBounds.max, quadBounds[idx].max);
    }

    return regionBounds;
}

size_t CoMesh::TriangleStorage::Reorder(
    size_t startIdx,
    size_t endIdx,
    std::function<bool(const CoAxisAlignedBoundingBox &)> comparator
) {
    std::vector<CoAxisAlignedBoundingBox> bounds = _ComputePrimitiveBounds(startIdx, endIdx);
    size_t splitIdx = endIdx;
    // partition
    for (size_t idx = startIdx; idx < endIdx;) {
        if (comparator(bounds[idx])) {
            ++idx;
        } else {
            --splitIdx;
            std::swap(_indices[idx], _indices[splitIdx]);
        }
    }

    return splitIdx;
}

bool CoMesh::TriangleStorage::PrimitivesIntersect(
    const CoRay &ray,
    size_t startIdx,
    size_t endIdx,
    IntersectionEvent &event
) const {
    for (size_t idx = startIdx; idx < endIdx; ++idx) {
        const vec4u &triangleIndex = _indices[startIdx];
        if (rayTriangleIntersection(
                ray,
                _positions[triangleIndex.x],
                _positions[triangleIndex.y],
                _positions[triangleIndex.z],
                event
            )) {
            return true;
        }
    }

    return false;
}

std::vector<CoAxisAlignedBoundingBox>
CoMesh::TriangleStorage::_ComputePrimitiveBounds(size_t startIdx, size_t endIdx) const {
    std::vector<CoAxisAlignedBoundingBox> bounds;
    bounds.reserve(endIdx - startIdx + 1);
    for (size_t idx = startIdx; idx < endIdx; ++idx) {
        const vec4u &triangleIndex = _indices[idx];
        const simd::vec3f boxMin12 = simd::min(_positions[triangleIndex.x], _positions[triangleIndex.y]);
        const simd::vec3f boxMin34 = simd::min(_positions[triangleIndex.z], _positions[triangleIndex.w]);
        const simd::vec3f boxMin = simd::min(boxMin12, boxMin34);

        const simd::vec3f boxMax12 = simd::max(_positions[triangleIndex.x], _positions[triangleIndex.y]);
        const simd::vec3f boxMax34 = simd::max(_positions[triangleIndex.z], _positions[triangleIndex.w]);
        const simd::vec3f boxMax = simd::max(boxMin12, boxMin34);

        bounds.push_back({
            .min = boxMin,
            .max = boxMax,
        });
    }

    return bounds;
}

CoMesh::QuadStorage::QuadStorage(simd::vec3f *positions, vec4u *indices, size_t numIndices)
    : _positions{positions}, _indices{indices}, _numIndices{numIndices} {
    _bounds = _ComputePrimitiveBounds(0, numIndices);
}

CoMesh::QuadStorage::~QuadStorage() {
    delete[] _positions;
    delete[] _indices;
}

size_t CoMesh::QuadStorage::NumPrimitives() const {
    return _numIndices;
}

CoAxisAlignedBoundingBox CoMesh::QuadStorage::PrimitiveBounds(size_t startIdx, size_t endIdx) const {
    static constexpr float minFloat = std::numeric_limits<float>::lowest();
    static constexpr float maxFloat = std::numeric_limits<float>::max();
    CoAxisAlignedBoundingBox regionBounds = {
        .min = simd::vec3f(maxFloat, maxFloat, maxFloat),
        .max = simd::vec3f(minFloat, minFloat, minFloat),
    };

    for (size_t idx = startIdx; idx < endIdx; ++idx) {
        regionBounds.min = simd::min(regionBounds.min, _bounds[idx].min);
        regionBounds.max = simd::max(regionBounds.max, _bounds[idx].max);
    }

    return regionBounds;
}

size_t CoMesh::QuadStorage::Reorder(
    size_t startIdx,
    size_t endIdx,
    std::function<bool(const CoAxisAlignedBoundingBox &)> comparator
) {
    size_t splitIdx = endIdx - 1;
    // partition
    for (size_t idx = startIdx; idx < splitIdx;) {
        if (comparator(_bounds[idx])) {
            ++idx;
        } else {
            --splitIdx;
            std::swap(_indices[idx], _indices[splitIdx]);
            std::swap(_bounds[idx], _bounds[splitIdx]);
        }
    }

    return splitIdx;
}

bool CoMesh::QuadStorage::PrimitivesIntersect(
    const CoRay &ray,
    size_t startIdx,
    size_t endIdx,
    IntersectionEvent &event
) const {
    bool hitPatch = false;
    for (size_t idx = startIdx; idx < endIdx; ++idx) {
        const vec4u &patchIndex = _indices[idx];
        hitPatch = rayPatchIntersection(
                       ray,
                       _positions[patchIndex.x],
                       _positions[patchIndex.y],
                       _positions[patchIndex.z],
                       _positions[patchIndex.w],
                       event
                   ) ||
                   hitPatch;
    }

    return hitPatch;
}

std::vector<CoAxisAlignedBoundingBox>
CoMesh::QuadStorage::_ComputePrimitiveBounds(size_t startIdx, size_t endIdx) const {
    std::vector<CoAxisAlignedBoundingBox> bounds;
    bounds.reserve(endIdx - startIdx + 1);
    for (size_t idx = startIdx; idx < endIdx; ++idx) {
        const vec4u &triangleIndex = _indices[idx];
        const simd::vec3f boxMin12 = simd::min(_positions[triangleIndex.x], _positions[triangleIndex.y]);
        const simd::vec3f boxMin34 = simd::min(_positions[triangleIndex.z], _positions[triangleIndex.w]);
        const simd::vec3f boxMin = simd::min(boxMin12, boxMin34);

        const simd::vec3f boxMax12 = simd::max(_positions[triangleIndex.x], _positions[triangleIndex.y]);
        const simd::vec3f boxMax34 = simd::max(_positions[triangleIndex.z], _positions[triangleIndex.w]);
        const simd::vec3f boxMax = simd::max(boxMin12, boxMin34);

        bounds.push_back({
            .min = boxMin,
            .max = boxMax,
        });
    }

    return bounds;
}

/// ----------------------------------- CoMesh -----------------------------------

std::shared_ptr<CoMesh> CoMesh::create(const CoMesh::CreateInfo &createInfo) {
    return std::shared_ptr<CoMesh>(new CoMesh(createInfo));
}

CoMesh::CoMesh(const CreateInfo &createInfo): _topology{createInfo.topology} {
    switch (_topology) {
    case CoPrimitiveTopology::kTriangle :
        _triangles = std::shared_ptr<TriangleStorage>(new TriangleStorage(createInfo.positions, createInfo.indices));
        _triangleAccelerator =
            std::unique_ptr<TriangleAccelerator>(new TriangleAccelerator(TriangleAccelerator::CreateWithPrimitivesInfo{
                .primitives = _triangles,
            }));
        break;
    case CoPrimitiveTopology::kQuad :
        _quads = std::shared_ptr<QuadStorage>(
            new QuadStorage(createInfo.positions, createInfo.indices, createInfo.numIndices)
        );
        _quadAccelerator =
            std::unique_ptr<QuadAccelerator>(new QuadAccelerator(QuadAccelerator::CreateWithPrimitivesInfo{
                .primitives = _quads,
            }));
    }
}

CoMesh::~CoMesh() {
}

bool CoMesh::intersects(const CoRay &ray, IntersectionEvent &intersectionEvent) {
    if (_topology == CoPrimitiveTopology::kTriangle) {
        return _triangleAccelerator->IntersectClosest(ray, intersectionEvent);
    } else {
        return _quadAccelerator->IntersectClosest(ray, intersectionEvent);
    }
}

template class CoBoundingVolume<CoMesh::TriangleStorage>;
template class CoBoundingVolume<CoMesh::QuadStorage>;

} // namespace cblt::geom
