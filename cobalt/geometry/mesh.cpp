#include "mesh.h"

#include "bounding_box.h"
#include "bounding_volume_mesh_storage.h"
#include "bounding_volume_types.h"
#include "intersection.h"

#include "core/logging.h"
#include "math/interpolation.h"
#include "math/math_types.h"
#include "math/simd/simd_vec3.h"

#include <limits>
#include <numeric>

namespace cblt::geom {

namespace {
template<typename T>
bool checkVertexAttributeBuffer(const CoMesh::VertexAttributeBuffer<T> &vertexBuffer) {
    if (!vertexBuffer.vertexCount || !vertexBuffer.vertices) {
        CoLogError("Vertex Buffer must be not empty");
        return false;
    }

    if (!(vertexBuffer.triangleIndices || vertexBuffer.patchIndices) ||
        !(vertexBuffer.triangleCount + vertexBuffer.patchCount)) {
        CoLogError("Vertex Buffer must contain face indices");
        return false;
    }

    if (bool(vertexBuffer.triangleIndices) != bool(vertexBuffer.triangleCount)) {
        CoLogError("Triangle Index buffer must declare both a valid pointer and number of elements");
        return false;
    }

    if (bool(vertexBuffer.patchIndices) != bool(vertexBuffer.patchCount)) {
        CoLogError("Patch Index buffer must declare both a valid pointer and number of elements");
        return false;
    }

    return true;
}

bool checkCreateInfo(const CoMesh::CreateInfo &createInfo) {
    return checkVertexAttributeBuffer(createInfo.positions);
}
} // namespace

std::shared_ptr<CoMesh> CoMesh::create(const CoMesh::CreateInfo &createInfo) {
    if (!checkCreateInfo(createInfo)) {
        return nullptr;
    }

    return std::shared_ptr<CoMesh>(new CoMesh(createInfo));
}

CoMesh::CoMesh(const CreateInfo &createInfo): _positions(createInfo.positions) {
    const std::shared_ptr<CoMeshStorage> meshStorage = std::make_shared<CoMeshStorage>(CoMeshStorage::VertexBuffer{
        .positions = _positions.vertices,
        .positionCount = _positions.vertexCount,
        .triangleIndices = _positions.triangleIndices,
        .triangleCount = _positions.triangleCount,
        .patchIndices = _positions.patchIndices,
        .patchCount = _positions.patchCount,
    });

    std::span<const vec3u> triangles = {
        _positions.triangleIndices.get(),
        _positions.triangleCount,
    };

    static constexpr float kMaxFloat = std::numeric_limits<float>::max();
    static constexpr float kMinFloat = std::numeric_limits<float>::lowest();
    _bounds = CoAxisAlignedBoundingBox{
        .min = {kMaxFloat, kMaxFloat, kMaxFloat},
        .max = {kMinFloat, kMinFloat, kMinFloat},
    };

    _bounds = std::accumulate(
        triangles.begin(),
        triangles.end(),
        _bounds,
        [positions = _positions.vertices](const CoAxisAlignedBoundingBox &bounds, const vec3u triangle) {
            return CoAxisAlignedBoundingBox{
                .min = simd::min(
                    simd::min(positions[triangle.x], positions[triangle.y]),
                    simd::min(bounds.min, positions[triangle.y])
                ),
                .max = simd::max(
                    simd::max(positions[triangle.x], positions[triangle.y]),
                    simd::max(bounds.max, positions[triangle.y])
                ),
            };
        }
    );

    std::span<const vec4u> patches = {
        _positions.patchIndices.get(),
        _positions.patchCount,
    };

    _bounds = std::accumulate(
        patches.begin(),
        patches.end(),
        _bounds,
        [positions = _positions.vertices](const CoAxisAlignedBoundingBox &bounds, const vec4u patch) {
            return CoAxisAlignedBoundingBox{
                .min = simd::min(
                    simd::min(
                        simd::min(positions[patch.x], positions[patch.y]),
                        simd::min(positions[patch.y], positions[patch.w])
                    ),
                    bounds.min
                ),
                .max = simd::max(
                    simd::max(
                        simd::max(positions[patch.x], positions[patch.y]),
                        simd::max(positions[patch.y], positions[patch.w])
                    ),
                    bounds.max
                ),
            };
        }
    );

    _accelerator = std::unique_ptr<MeshAccelerator>(new MeshAccelerator(MeshAccelerator::CreateWithPrimitivesInfo{
        .primitives = meshStorage,
    }));
}

CoAxisAlignedBoundingBox CoMesh::bounds() const {
    return _bounds;
}

geom::IntersectionResult CoMesh::intersects(const CoRay &ray) const {
    assert(_accelerator && "missing accelerator");
    return _accelerator->intersects(ray);
}

} // namespace cblt::geom
