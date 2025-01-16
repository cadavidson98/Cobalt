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

CoMesh::MeshStorage::MeshStorage(simd::vec3f *positions, vec4u *indices, size_t numFaces)
    : CoPrimitiveStorage(numFaces), _positions{positions}, _indices{indices}, _numIndices{numFaces} {
    _bounds = _ComputePrimitiveBounds(0, numFaces);
}

CoMesh::MeshStorage::~MeshStorage() {
    delete[] _positions;
    delete[] _indices;
}

size_t CoMesh::MeshStorage::NumPrimitives() const {
    return _numIndices;
}

CoAxisAlignedBoundingBox CoMesh::MeshStorage::PrimitiveBounds(size_t startIdx, size_t endIdx) const {
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

size_t CoMesh::MeshStorage::Reorder(
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
            std::swap(_primitiveIndices[idx], _primitiveIndices[splitIdx]);
        }
    }

    return splitIdx;
}

bool CoMesh::MeshStorage::PrimitivesIntersect(
    const CoRay &ray,
    size_t startIdx,
    size_t endIdx,
    IntersectionEvent &event
) const {
    bool hitFace = false;
    float timeMin(std::numeric_limits<float>::max()), timeMax(std::numeric_limits<float>::max());
    vec2f hitCoordinates = {0.f, 0.f};
    for (size_t idx = startIdx; idx < endIdx; ++idx) {
        const vec4u &faceIndex = _indices[idx];
        if (faceIndex.w != kInvalidIndex) {
            const bool hitPatch = rayPatchIntersection(
                ray,
                _positions[faceIndex.x],
                _positions[faceIndex.y],
                _positions[faceIndex.z],
                _positions[faceIndex.w],
                timeMin, timeMax, hitCoordinates);
            if (hitPatch && timeMin < event.timeMin) {
                event.timeMin = timeMin;
                event.localCoordinates = hitCoordinates;
                event.primitiveIndex = _primitiveIndices[idx];
                hitFace = true;
            }
        } else {
            const bool hitTriangle = rayTriangleIntersection(
                ray,
                _positions[faceIndex.x],
                _positions[faceIndex.y],
                _positions[faceIndex.z],
                timeMin, hitCoordinates);
            if (hitTriangle && timeMin < event.timeMin) {
                event.timeMin = timeMin;
                event.localCoordinates = hitCoordinates;
                event.primitiveIndex = _primitiveIndices[idx];
                hitFace = true;
            }
        }
    }

    return hitFace;
}

std::vector<CoAxisAlignedBoundingBox>
CoMesh::MeshStorage::_ComputePrimitiveBounds(size_t startIdx, size_t endIdx) const {
    std::vector<CoAxisAlignedBoundingBox> bounds;
    bounds.reserve(endIdx - startIdx + 1);
    for (size_t idx = startIdx; idx < endIdx; ++idx) {
        simd::vec3f boxMin;
        simd::vec3f boxMax;

        const vec4u &faceIndex = _indices[idx];

        if (faceIndex.w != kInvalidIndex) { 
            const simd::vec3f boxMin12 = simd::min(_positions[faceIndex.x], _positions[faceIndex.y]);
            const simd::vec3f boxMin34 = simd::min(_positions[faceIndex.z], _positions[faceIndex.w]);
            boxMin = simd::min(boxMin12, boxMin34);

            const simd::vec3f boxMax12 = simd::max(_positions[faceIndex.x], _positions[faceIndex.y]);

            const simd::vec3f boxMax34 = simd::max(_positions[faceIndex.z], _positions[faceIndex.w]);
            boxMax = simd::max(boxMin12, boxMin34);
        } else {
            const simd::vec3f boxMin12 = simd::min(_positions[faceIndex.x], _positions[faceIndex.y]);
            boxMin = simd::min(boxMin12, _positions[faceIndex.z]);

            const simd::vec3f boxMax12 = simd::max(_positions[faceIndex.x], _positions[faceIndex.y]);
            boxMax = simd::max(boxMin12, _positions[faceIndex.z]);
        }

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

CoMesh::CoMesh(const CreateInfo &createInfo) {
    _primitives = std::shared_ptr<MeshStorage>(new MeshStorage(createInfo.positions, createInfo.indices, createInfo.numIndices));
    _accelerator = std::unique_ptr<MeshAccelerator>(new MeshAccelerator(MeshAccelerator::CreateWithPrimitivesInfo{
        .primitives = _primitives,
    }));
}

CoMesh::~CoMesh() {
}

bool CoMesh::intersects(const CoRay &ray, IntersectionEvent &intersectionEvent) {
    return _accelerator->IntersectClosest(ray, intersectionEvent);
}

CoSurface CoMesh::resolveSurface(const IntersectionEvent &intersectionEvent) {
    const vec4u primitiveIndices = _primitives->_indices[intersectionEvent.primitiveIndex];
    if (primitiveIndices.w != uint32_t(-1)) {
        return interpolatePatch(
            _primitives->_positions[primitiveIndices.x],
            _primitives->_positions[primitiveIndices.y],
            _primitives->_positions[primitiveIndices.z],
            _primitives->_positions[primitiveIndices.w],
            intersectionEvent.localCoordinates);
    }
    return CoSurface();
}

} // namespace cblt::geom
