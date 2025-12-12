#include "mesh.h"

#include "bounding_box.h"
#include "bounding_volume_mesh_storage.h"
#include "bounding_volume_types.h"
#include "intersection.h"

#include "core/logging.h"
#include "math/math_types.h"
#include "math/simd/simd_vec3.h"

#include <limits>
#include <numeric>

namespace cblt::geom {

namespace {

constexpr float kMaxFloat = std::numeric_limits<float>::max();
constexpr float kMinFloat = std::numeric_limits<float>::lowest();

template<typename T>
[[nodiscard]] bool checkVertexAttributeBuffer(const core::VertexAttributeBuffer<T> &vertexBuffer) {
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

[[nodiscard]] bool checkCreateInfo(const CoMesh::CreateInfo &createInfo) {
    return checkVertexAttributeBuffer(createInfo.positions);
}
} // anonymous namespace

std::shared_ptr<CoMesh> CoMesh::create(const CoMesh::CreateInfo &createInfo) {
    if (!checkCreateInfo(createInfo)) {
        return nullptr;
    }

    const core::VertexAttributeBuffer<simd::vec3f> &positions = createInfo.positions;

    const std::span<const vec3u> triangles = {
        positions.triangleIndices.get(),
        positions.triangleCount,
    };

    const CoAxisAlignedBoundingBox triangleBounds = std::accumulate(
        triangles.begin(),
        triangles.end(),
        CoAxisAlignedBoundingBox{.min = simd::vec3f(kMaxFloat), .max = simd::vec3f(kMinFloat)},
        [positions = positions.vertices](const CoAxisAlignedBoundingBox &bounds, const vec3u triangle) {
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

    const std::span<const vec4u> patches = {
        positions.patchIndices.get(),
        positions.patchCount,
    };

    const CoAxisAlignedBoundingBox patchBounds = std::accumulate(
        patches.begin(),
        patches.end(),
        CoAxisAlignedBoundingBox{.min = simd::vec3f(kMaxFloat), .max = simd::vec3f(kMinFloat)},
        [positions = positions.vertices](const CoAxisAlignedBoundingBox &bounds, const vec4u patch) {
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

    // TODO: do I need to check for 'Max' when computing mins (and mins when computing Max)
    const CoAxisAlignedBoundingBox meshBounds = {
        .min = simd::min(patchBounds.min, triangleBounds.min),
        .max = simd::max(patchBounds.max, triangleBounds.max),
    };

    auto mortonKeyer = [](const geom::MortonPrimitive &lhs) {
        return lhs.mortonCode;
    };

    const std::shared_ptr<CoMeshStorage> meshStorage = std::make_shared<CoMeshStorage>(positions);

    std::vector<geom::MortonPrimitive> mortonEncodedPrimitives = meshStorage->mortonEncodePrimitives();

    core::radix_sort<30>(mortonEncodedPrimitives.begin(), mortonEncodedPrimitives.end(), mortonKeyer);

    // TODO: structure of array here; looks like I can "coarsen these reorders"
    meshStorage->reorder(mortonEncodedPrimitives);

    return std::shared_ptr<CoMesh>(new CoMesh(meshStorage, mortonEncodedPrimitives, meshBounds));
}

// there is a "function with side effects" assumption here
CoMesh::CoMesh(
    std::shared_ptr<CoMeshStorage> meshStorage,
    std::span<const MortonPrimitive> meshPrimitives,
    CoAxisAlignedBoundingBox bounds
)
    : _accelerator(meshStorage, meshPrimitives), _bounds{bounds} {
}

CoAxisAlignedBoundingBox CoMesh::bounds() const {
    return _bounds;
}

geom::IntersectionResult CoMesh::intersects(const CoRay &ray) const {
    return _accelerator.intersects(ray);
}

} // namespace cblt::geom
