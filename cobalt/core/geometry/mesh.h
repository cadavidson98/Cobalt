#ifndef CBLT_CORE_MESH_H
#define CBLT_CORE_MESH_H

#include "simd/simd_vec3.h"
#include "vec3.h"

#include <cstdint>
#include <fstream>
#include <memory>
#include <optional>
#include <string>

namespace cblt::geom {

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

    private:
        struct VertexBuffer {
                simd::vec3f *vertices;
                size_t numVertices;
                uint32_t *indices;
                size_t numIndices;

                VertexBuffer(size_t numVertices, size_t numIndices);
        };

        struct CreateFromBuffersInfo {
                VertexBuffer positions;
                VertexBuffer normals;
                CoPrimitiveTopology topology;
        };

        struct MeshBuffersSizeInfo {
                size_t numPositions;
                size_t numNormals;
                size_t numIndices;
                CoPrimitiveTopology topology;
        };

        VertexBuffer _positions;
        VertexBuffer _normals;
        CoPrimitiveTopology _topology;

        static bool _checkCreateInfo(const CreateFromFileInfo &createInfo);
        static std::optional<CreateFromBuffersInfo> _readObjFile(const std::string &fileName);
        static std::optional<MeshBuffersSizeInfo> _scanVertexBuffersSize(std::ifstream &objFileStream);
        static vec3i _parseIndices(const std::string &faceString);

        CoMesh() = delete;
        CoMesh(const CreateFromBuffersInfo &createInfo);
};

} // namespace cblt::geom

#endif // CBLT_CORE_MESH_H
