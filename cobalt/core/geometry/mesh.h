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
        struct CreateFromFileInfo {
                std::string fileName;
                std::string fileExtension;
        };

        static std::shared_ptr<CoMesh> create(const CreateFromFileInfo &createInfo);

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

        struct CreateFromBuffersInfo {
                simd::vec3f *positions;
                size_t numVertices;
                vec4u *indices;
                size_t numIndices;
                CoPrimitiveTopology topology;
        };

        struct MeshBuffersSizeInfo {
                size_t numPositions;
                size_t numNormals;
                size_t numFaces;
                CoPrimitiveTopology topology;
        };

        using TriangleAccelerator = CoBoundingVolume<TriangleStorage>;
        using QuadAccelerator = CoBoundingVolume<QuadStorage>;

        CoPrimitiveTopology _topology;

        std::shared_ptr<TriangleStorage> _triangles;
        std::unique_ptr<TriangleAccelerator> _triangleAccelerator;

        std::shared_ptr<QuadStorage> _quads;
        std::unique_ptr<QuadAccelerator> _quadAccelerator;

        static bool _checkCreateInfo(const CreateFromFileInfo &createInfo);
        static std::optional<CreateFromBuffersInfo> _readObjFile(const std::string &fileName);
        static std::optional<MeshBuffersSizeInfo> _scanMeshBuffersSize(std::ifstream &objFileStream);
        static vec3i _parseIndices(const std::string &faceString);

        CoMesh() = delete;
        CoMesh(const CreateFromBuffersInfo &createInfo);
};

} // namespace cblt::geom

#endif // CBLT_CORE_MESH_H
