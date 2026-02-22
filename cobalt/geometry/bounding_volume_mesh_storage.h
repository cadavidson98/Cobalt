#ifndef CBLT_GEOM_BOUNDING_VOLUME_MESH_STORAGE_H
#define CBLT_GEOM_BOUNDING_VOLUME_MESH_STORAGE_H

#include "bounding_box.h"
#include "bounding_volume_types.h"
#include "intersection.h"

#include "core/morton_encoding.h"
#include "core/size_types.h"
#include "core/vertex_buffer.h"
#include "math/simd/simd_vec3.h"

#include <algorithm>
#include <limits>

namespace cblt::geom {

class MeshStorage {
public:
    struct Extents {
        PrimitiveExtent triangles;
        PrimitiveExtent patches;
    };

    MeshStorage(core::VertexAttributeBuffer<simd::vec3f> buffer): _positionsBuffer(buffer) {
        static constexpr float kMinFloat = std::numeric_limits<float>::lowest();
        static constexpr float kMaxFloat = std::numeric_limits<float>::max();
        _bounds = AxisAlignedBoundingBox{
            .min = {kMaxFloat, kMaxFloat, kMaxFloat},
            .max = {kMinFloat, kMinFloat, kMinFloat},
        };

        auto computeTriangleBounds = [](const simd::vec3f &A, const simd::vec3f &B, const simd::vec3f &C) {
            return AxisAlignedBoundingBox{
                .min = simd::min(simd::min(A, B), C),
                .max = simd::max(simd::max(A, B), C),
            };
        };

        for (size_t triangleIdx = 0; triangleIdx < _positionsBuffer.triangleCount; ++triangleIdx) {
            const vec3u &triangle = _positionsBuffer.triangleIndices[triangleIdx];

            const AxisAlignedBoundingBox bounds = computeTriangleBounds(
                _positionsBuffer.vertices[triangle.x],
                _positionsBuffer.vertices[triangle.y],
                _positionsBuffer.vertices[triangle.z]
            );
            _bounds = AxisAlignedBoundingBox::Union(_bounds, bounds);
        }

        auto computePatchBounds = [](const simd::vec3f &A,
                                     const simd::vec3f &B,
                                     const simd::vec3f &C,
                                     const simd::vec3f &D) -> AxisAlignedBoundingBox {
            return AxisAlignedBoundingBox{
                .min = simd::min(simd::min(A, B), simd::min(C, D)),
                .max = simd::max(simd::max(A, B), simd::max(C, D)),
            };
        };

        for (size_t patchIdx = 0; patchIdx < _positionsBuffer.patchCount; ++patchIdx) {
            const vec4u &patch = _positionsBuffer.patchIndices[patchIdx];

            const AxisAlignedBoundingBox bounds = computePatchBounds(
                _positionsBuffer.vertices[patch.x],
                _positionsBuffer.vertices[patch.y],
                _positionsBuffer.vertices[patch.z],
                _positionsBuffer.vertices[patch.w]
            );

            _bounds = AxisAlignedBoundingBox::Union(_bounds, bounds);
        }
    }

    // To be pulled out into a new interface prior to storage creation
    [[nodiscard]] std::vector<MortonPrimitive> mortonEncodePrimitives() const {
        const AxisAlignedBoundingBox primitiveBounds = bounds();
        const simd::vec3f boundsExtent = primitiveBounds.Scales();
        const simd::vec3f boundsMin = primitiveBounds.min;

        std::vector<MortonPrimitive> primitives;
        primitives.reserve(_positionsBuffer.triangleCount + _positionsBuffer.patchCount);

        auto computeMortonCode = [&boundsExtent, &boundsMin](const AxisAlignedBoundingBox &boundingBox) -> uint32_t {
            static constexpr float kFloatToUint = float((1 << 10) - 1);
            const simd::vec3f normalizedPosition = (boundingBox.Center() - boundsMin) / boundsExtent;
            const std::array<float, 4> values = (kFloatToUint * normalizedPosition).Values();
            return core::mortonEncode(values[0], values[1], values[2]);
        };

        auto computeTriangleBounds = [](const simd::vec3f &A, const simd::vec3f &B, const simd::vec3f &C) {
            return AxisAlignedBoundingBox{
                .min = simd::min(simd::min(A, B), C),
                .max = simd::max(simd::max(A, B), C),
            };
        };

        for (size_t triangleIdx = 0; triangleIdx < _positionsBuffer.triangleCount; ++triangleIdx) {
            const vec3u &triangle = _positionsBuffer.triangleIndices[triangleIdx];

            const AxisAlignedBoundingBox boundingBox = computeTriangleBounds(
                _positionsBuffer.vertices[triangle.x],
                _positionsBuffer.vertices[triangle.y],
                _positionsBuffer.vertices[triangle.z]
            );

            primitives.push_back(
                MortonPrimitive{
                    .mortonCode = computeMortonCode(boundingBox),
                    .primitive =
                        Primitive{
                                  .type = PrimitiveType::kTriangle,
                                  .index = uint32_t(triangleIdx),
                                  },
                    .boundingBox = boundingBox,
            }
            );
        }

        auto computePatchBounds = [](const simd::vec3f &A,
                                     const simd::vec3f &B,
                                     const simd::vec3f &C,
                                     const simd::vec3f &D) -> AxisAlignedBoundingBox {
            return AxisAlignedBoundingBox{
                .min = simd::min(simd::min(A, B), simd::min(C, D)),
                .max = simd::max(simd::max(A, B), simd::max(C, D)),
            };
        };

        for (size_t patchIdx = 0; patchIdx < _positionsBuffer.patchCount; ++patchIdx) {
            const vec4u &patch = _positionsBuffer.patchIndices[patchIdx];

            const AxisAlignedBoundingBox boundingBox = computePatchBounds(
                _positionsBuffer.vertices[patch.x],
                _positionsBuffer.vertices[patch.y],
                _positionsBuffer.vertices[patch.z],
                _positionsBuffer.vertices[patch.w]
            );

            primitives.push_back(
                MortonPrimitive{
                    .mortonCode = computeMortonCode(boundingBox),
                    .primitive =
                        Primitive{
                                  .type = PrimitiveType::kPatch,
                                  .index = uint32_t(patchIdx),
                                  },
                    .boundingBox = boundingBox,
            }
            );
        }

        return primitives;
    }

    void reorder(std::span<MortonPrimitive> primitives) {
        uint32_t triangleIdx = 0;
        uint32_t patchIdx = 0;

        auto copyArray = []<typename T>(T *vals, size_t count) -> std::vector<T> {
            std::span<const T> source = {vals, count};
            return std::vector<T>(source.begin(), source.end());
        };

        const std::vector<vec3u> trianglesCopy =
            copyArray(_positionsBuffer.triangleIndices.get(), _positionsBuffer.triangleCount);
        const std::vector<vec4u> patchesCopy =
            copyArray(_positionsBuffer.patchIndices.get(), _positionsBuffer.patchCount);

        for (MortonPrimitive &mortonPrimitive : primitives) {
            switch (mortonPrimitive.primitive.type) {
            case PrimitiveType::kTriangle : {
                const size_t newIdx = triangleIdx++;
                _positionsBuffer.triangleIndices[newIdx] = trianglesCopy[mortonPrimitive.primitive.index];
                mortonPrimitive.primitive.index = newIdx;
                continue;
            }
            case PrimitiveType::kPatch : {
                const size_t newIdx = patchIdx++;
                _positionsBuffer.patchIndices[newIdx] = patchesCopy[mortonPrimitive.primitive.index];
                mortonPrimitive.primitive.index = newIdx;
                continue;
            }
            default :
                assert(false);
            }
        }
    }

    // Bounding Volume Private methods

    [[nodiscard]] Extents findExtents(std::span<const MortonPrimitive> primitives) const {
        const auto expandExtent = [](const PrimitiveExtent extent, const Primitive &primitive) -> PrimitiveExtent {
            return {
                .start = std::min(extent.start, primitive.index),
                .count = extent.count + 1,
            };
        };

        Extents extents;

        for (const MortonPrimitive &mortonPrimitive : primitives) {
            const Primitive &primitive = mortonPrimitive.primitive;
            switch (primitive.type) {
            case PrimitiveType::kTriangle :
                extents.triangles = expandExtent(extents.triangles, primitive);
                continue;
            case PrimitiveType::kPatch :
                extents.patches = expandExtent(extents.patches, primitive);
                continue;

            default :
                assert(false);
            }
        }

        return extents;
    }

    [[nodiscard]] IntersectionResult intersects(const Extents &extents, const Ray &ray) const {
        IntersectionResult result;

        const PrimitiveExtent &triangles = extents.triangles;
        for (size_t idx = triangles.start; idx < triangles.start + triangles.count; ++idx) {
            float localTimeMin = std::numeric_limits<float>::max();
            const vec3u &triangle = _positionsBuffer.triangleIndices[idx];
            vec2f coordinates;

            const bool hitTriangle = rayTriangleIntersection(
                ray,
                _positionsBuffer.vertices[triangle.x],
                _positionsBuffer.vertices[triangle.y],
                _positionsBuffer.vertices[triangle.z],
                localTimeMin,
                coordinates
            );

            if (hitTriangle && localTimeMin < result.hitTime) {
                result.hitTime = localTimeMin;
                result.primitive = Primitive{
                    .type = PrimitiveType::kTriangle,
                    .index = uint32_t(idx),
                };
            }
        }

        const PrimitiveExtent &patches = extents.patches;
        for (size_t idx = patches.start; idx < patches.start + patches.count; ++idx) {
            float localTimeMin = std::numeric_limits<float>::max();
            float localTimeMax = std::numeric_limits<float>::max();
            const vec4u &patch = _positionsBuffer.patchIndices[idx];

            vec2f coordinates;

            const bool hitPatch = rayPatchIntersection(
                ray,
                _positionsBuffer.vertices[patch.x],
                _positionsBuffer.vertices[patch.y],
                _positionsBuffer.vertices[patch.z],
                _positionsBuffer.vertices[patch.w],
                localTimeMin,
                localTimeMax,
                coordinates
            );

            if (hitPatch && localTimeMin < result.hitTime) {
                result.hitTime = localTimeMin;
                result.primitive = {
                    .type = PrimitiveType::kPatch,
                    .index = uint32_t(idx),
                };
            }
        }

        return result;
    }

private:
    AxisAlignedBoundingBox _bounds;

    core::VertexAttributeBuffer<simd::vec3f> _positionsBuffer;

    AxisAlignedBoundingBox bounds() const {
        return _bounds;
    }
};

template<>
struct storageExtent<MeshStorage> {
    using value = MeshStorage::Extents;
};

} // namespace cblt::geom

#endif // CBLT_GEOM_BOUNDING_VOLUME_MESH_STORAGE_H
