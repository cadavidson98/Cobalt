#ifndef CBLT_GEOM_BOUNDING_VOLUME_SCENE_STORAGE_H
#define CBLT_GEOM_BOUNDING_VOLUME_SCENE_STORAGE_H

#include "bounding_box.h"
#include "bounding_volume_types.h"
#include "mesh.h"
#include "sphere.h"

#include "core/morton_encoding.h"
#include "math/simd/simd_vec3.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>

namespace cblt::geom {

class CoSceneStorage {
public:
    struct Extents {
        PrimitiveExtent spheres;
        PrimitiveExtent boxes;
        PrimitiveExtent meshes;
    };

    CoSceneStorage() {
        static constexpr float kMinValue = std::numeric_limits<float>::lowest();
        static constexpr float kMaxValue = std::numeric_limits<float>::max();
        _bounds = {
            .min = simd::vec3f(kMaxValue, kMaxValue, kMaxValue),
            .max = simd::vec3f(kMinValue, kMinValue, kMinValue),
        };
    }

    // MARK: adding primitives methods
    void addSphere(const CoSphere &sphere) {
        _spherePrimitives.push_back(sphere);
        const CoAxisAlignedBoundingBox sphereBounds = {
            .min = sphere.center - simd::vec3f(sphere.radius, sphere.radius, sphere.radius),
            .max = sphere.center + simd::vec3f(sphere.radius, sphere.radius, sphere.radius),
        };

        _bounds = CoAxisAlignedBoundingBox::Union(_bounds, sphereBounds);
    }

    void addMesh(const std::shared_ptr<CoMesh> mesh) {
        _meshPrimitives.push_back(mesh);
        _bounds = CoAxisAlignedBoundingBox::Union(_bounds, mesh->bounds());
    }

    // MARK: bounding volume helper methods
    void reorder(std::span<MortonPrimitive> primitives) {
        uint32_t sphereIdx = 0;
        uint32_t meshIdx = 0;

        std::vector<CoSphere> spheresCopy = _spherePrimitives;
        std::vector<std::shared_ptr<CoMesh>> meshesCopy = _meshPrimitives;

        for (MortonPrimitive &mortonPrimitive : primitives) {
            switch (mortonPrimitive.primitive.type) {
            case PrimitiveType::kSphere : {
                const size_t newIdx = sphereIdx++;
                _spherePrimitives[newIdx] = spheresCopy[mortonPrimitive.primitive.index];
                mortonPrimitive.primitive.index = newIdx;
                continue;
            }
            case PrimitiveType::kMesh : {
                const size_t newIdx = meshIdx++;
                _meshPrimitives[newIdx] = meshesCopy[mortonPrimitive.primitive.index];
                mortonPrimitive.primitive.index = newIdx;
                continue;
            };
            case PrimitiveType::kPatch : [[fallthrough]];
            case PrimitiveType::kTriangle : [[fallthrough]];
            default : assert(false);
            }
        }
    }

    [[nodiscard]] std::vector<MortonPrimitive> mortonEncodePrimitives() const {
        const CoAxisAlignedBoundingBox primitiveBounds = bounds();
        const simd::vec3f boundsExtent = primitiveBounds.Scales();
        const simd::vec3f boundsMin = primitiveBounds.min;

        std::vector<MortonPrimitive> primitives;
        primitives.reserve(_spherePrimitives.size() + _meshPrimitives.size());

        auto encodePrimitive = [&boundsExtent, &boundsMin](const CoAxisAlignedBoundingBox &boundingBox) -> uint32_t {
            static constexpr float kFloatToUint = float((1 << 10) - 1);
            const simd::vec3f normalizedPosition = (boundingBox.Center() - boundsMin) / boundsExtent;
            const std::array<float, 4> values = (kFloatToUint * normalizedPosition).Values();
            return core::mortonEncode(values[0], values[1], values[2]);
        };

        for (size_t index = 0; index < _spherePrimitives.size(); ++index) {
            const CoSphere &sphere = _spherePrimitives[index];
            const CoAxisAlignedBoundingBox boundingBox = {
                .min = sphere.center - simd::vec3f(sphere.radius),
                .max = sphere.center + simd::vec3f(sphere.radius),
            };

            primitives.push_back(MortonPrimitive{
                .mortonCode = encodePrimitive(boundingBox),
                .primitive =
                    {
                                .type = PrimitiveType::kSphere,
                                .index = uint32_t(index),
                                },
                .boundingBox = boundingBox,
            });
        }

        for (size_t index = 0; index < _meshPrimitives.size(); ++index) {
            const CoMesh &mesh = *_meshPrimitives[index];
            const CoAxisAlignedBoundingBox boundingBox = mesh.bounds();
            primitives.push_back(MortonPrimitive{
                .mortonCode = encodePrimitive(boundingBox),
                .primitive =
                    {
                                .type = PrimitiveType::kMesh,
                                .index = uint32_t(index),
                                },
                .boundingBox = boundingBox,
            });
        }

        return primitives;
    }

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
            case PrimitiveType::kSphere : extents.spheres = expandExtent(extents.spheres, primitive); continue;
            case PrimitiveType::kMesh : extents.meshes = expandExtent(extents.meshes, primitive); continue;
            default : assert(false);
            }
        }

        return extents;
    }

    [[nodiscard]] IntersectionResult intersects(const Extents &extents, const CoRay &ray) {
        IntersectionResult result;

        const PrimitiveExtent &spheres = extents.spheres;
        for (size_t idx = spheres.start; idx < spheres.start + spheres.count; ++idx) {
            float localTimeMin = std::numeric_limits<float>::max();
            float localTimeMax = std::numeric_limits<float>::max();
            const CoSphere &sphere = _spherePrimitives[idx];
            if (raySphereIntersection(ray, sphere, localTimeMin, localTimeMax) && localTimeMin < result.hitTime) {
                result.hitTime = localTimeMin;
                result.primitive = {
                    .type = PrimitiveType::kSphere,
                    .index = uint32_t(idx),
                };
            }
        }

        const PrimitiveExtent &meshes = extents.meshes;
        for (size_t idx = meshes.start; idx < meshes.start + meshes.count; ++idx) {
            const CoMesh &mesh = *_meshPrimitives[idx];
            const IntersectionResult meshResult = mesh.intersects(ray);
            if (meshResult.primitive.type != kNone && meshResult.hitTime < result.hitTime) {
                result.hitTime = meshResult.hitTime;
                result.primitive = {
                    .type = PrimitiveType::kMesh,
                    .index = uint32_t(idx),
                };
            }
        }

        return result;
    }

private:
    CoAxisAlignedBoundingBox _bounds;

    std::vector<cblt::geom::CoSphere> _spherePrimitives;
    std::vector<std::shared_ptr<cblt::geom::CoMesh>> _meshPrimitives;

    CoAxisAlignedBoundingBox bounds() const {
        return _bounds;
    }
};

template<>
struct storageExtent<CoSceneStorage> {
    using value = CoSceneStorage::Extents;
};

} // namespace cblt::geom

#endif // CBLT_GEOM_BOUNDING_VOLUME_SCENE_STORAGE_H
