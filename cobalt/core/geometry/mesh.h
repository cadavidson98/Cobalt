#ifndef CBLT_CORE_MESH_H
#define CBLT_CORE_MESH_H

#include "bounding_volume.h"
#include "simd/simd_vec3.h"
#include "vec3.h"
#include "vec4.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace cblt::geom {

struct CoRay;
struct IntersectionEvent;

enum class CoPrimitiveTopology {
    kTriangle,
    kQuad,
};

class CoMesh {
    public:
        struct CreateInfo {
                simd::vec3f *positions;
                size_t numVertices;
                vec4u *indices;
                size_t numIndices;
                CoPrimitiveTopology topology;
        };

        static std::shared_ptr<CoMesh> create(const CreateInfo &createInfo);

        ~CoMesh();

        bool intersects(const CoRay &ray, IntersectionEvent &intersectionEvent);

    private:
        class TriangleStorage {
            public:
                TriangleStorage(simd::vec3f *positions, vec4u *indices);
                ~TriangleStorage();

                size_t NumPrimitives() const;
                CoAxisAlignedBoundingBox PrimitiveBounds(size_t startIdx, size_t endIdx) const;
                size_t Reorder(
                    size_t startIdx,
                    size_t endIdx,
                    std::function<bool(const CoAxisAlignedBoundingBox &)> comparator
                );
                bool
                PrimitivesIntersect(const CoRay &ray, size_t startIdx, size_t endIdx, IntersectionEvent &event) const;

            private:
                vec4u *_indices;
                size_t numIndices;

                simd::vec3f *_positions;

                std::vector<CoAxisAlignedBoundingBox> _ComputePrimitiveBounds(size_t startIdx, size_t endIdx) const;
        };

        class QuadStorage {
            public:
                QuadStorage(simd::vec3f *positions, vec4u *indices, size_t numIndices);
                ~QuadStorage();

                size_t NumPrimitives() const;
                CoAxisAlignedBoundingBox PrimitiveBounds(size_t startIdx, size_t endIdx) const;
                size_t Reorder(
                    size_t startIdx,
                    size_t endIdx,
                    std::function<bool(const CoAxisAlignedBoundingBox &)> comparator
                );
                bool
                PrimitivesIntersect(const CoRay &ray, size_t startIdx, size_t endIdx, IntersectionEvent &event) const;

            private:
                vec4u *_indices;
                size_t _numIndices;

                simd::vec3f *_positions;

                std::vector<CoAxisAlignedBoundingBox> _bounds;

                std::vector<CoAxisAlignedBoundingBox> _ComputePrimitiveBounds(size_t startIdx, size_t endIdx) const;
        };

        using TriangleAccelerator = CoBoundingVolume<TriangleStorage>;
        using QuadAccelerator = CoBoundingVolume<QuadStorage>;

        CoPrimitiveTopology _topology;

        std::shared_ptr<TriangleStorage> _triangles;
        std::unique_ptr<TriangleAccelerator> _triangleAccelerator;

        std::shared_ptr<QuadStorage> _quads;
        std::unique_ptr<QuadAccelerator> _quadAccelerator;

        CoMesh() = delete;
        CoMesh(const CreateInfo &createInfo);
};

} // namespace cblt::geom

#endif // CBLT_CORE_MESH_H
