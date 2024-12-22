#ifndef CBLT_CORE_MESH_H
#define CBLT_CORE_MESH_H

#include "bounding_volume.h"
#include "simd/simd_vec3.h"
#include "vec3.h"
#include "vec4.h"

#include <cstdint>
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
        struct MeshTriangle {
                vec3u indices;
                simd::vec3f *vertices;
        };

        struct MeshQuad {
                vec4u indices;
                simd::vec3f *vertices;
        };

        struct MeshTriangleIntersector {
                const CoRay ray;
                IntersectionEvent event;
                bool operator()(const MeshTriangle &meshTriangle);
        };

        struct MeshQuadIntersector {
                const CoRay ray;
                IntersectionEvent event;
                bool operator()(const MeshQuad &meshQuad);
        };

        struct MeshTriangleBounder {
                CoAxisAlignedBoundingBox operator()(const MeshTriangle &meshTriangle);
        };

        struct MeshQuadBounder {
                CoAxisAlignedBoundingBox operator()(const MeshQuad &meshQuad);
        };

        struct VertexBuffer {
                simd::vec3f *vertices;
                size_t numVertices;

                VertexBuffer(size_t numVertices);
        };

        struct IndexBuffer {
                union {
                        MeshTriangle *triangles;
                        MeshQuad *quads;
                };
                size_t numPrimitives;

                IndexBuffer(size_t numPrimtives, CoPrimitiveTopology topology);
        };

        struct CreateFromBuffersInfo {
                VertexBuffer positions;
                IndexBuffer indices;
                VertexBuffer normals;
                CoPrimitiveTopology topology;
        };

        struct MeshBuffersSizeInfo {
                size_t numPositions;
                size_t numNormals;
                size_t numFaces;
                CoPrimitiveTopology topology;
        };

        using TriangleAccelerator = CoBoundingVolume<MeshTriangle, MeshTriangleBounder, MeshTriangleIntersector>;
        using QuadAccelerator = CoBoundingVolume<MeshQuad, MeshQuadBounder, MeshQuadIntersector>;

        VertexBuffer _positions;
        IndexBuffer _primitives;
        VertexBuffer _normals;
        CoPrimitiveTopology _topology;

        std::unique_ptr<TriangleAccelerator> _triangles;
        std::unique_ptr<QuadAccelerator> _quads;

        static bool _checkCreateInfo(const CreateFromFileInfo &createInfo);
        static std::optional<CreateFromBuffersInfo> _readObjFile(const std::string &fileName);
        static std::optional<MeshBuffersSizeInfo> _scanMeshBuffersSize(std::ifstream &objFileStream);
        static vec3i _parseIndices(const std::string &faceString);

        CoMesh() = delete;
        CoMesh(const CreateFromBuffersInfo &createInfo);
};

} // namespace cblt::geom

#endif // CBLT_CORE_MESH_H
