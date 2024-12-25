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

CoMesh::VertexBuffer::VertexBuffer(size_t _numVertices): numVertices{_numVertices}, vertices{nullptr} {
    vertices = new simd::vec3f[numVertices];
}

CoMesh::IndexBuffer::IndexBuffer(size_t _numTriangles, CoPrimitiveTopology primitiveType)
    : numPrimitives{_numTriangles} {
    if (primitiveType == CoPrimitiveTopology::kTriangle) {
        triangles = new MeshTriangle[numPrimitives];
    } else {
        quads = new MeshQuad[numPrimitives];
    }
}

bool CoMesh::MeshTriangleIntersector::operator()(const MeshTriangle &meshTriangle) {
    return rayTriangleIntersection(
        ray,
        meshTriangle.vertices[meshTriangle.indices.x],
        meshTriangle.vertices[meshTriangle.indices.y],
        meshTriangle.vertices[meshTriangle.indices.z],
        event
    );
}

CoAxisAlignedBoundingBox CoMesh::MeshTriangleBounder::operator()(const MeshTriangle &meshTriangle) {
    simd::vec3f boxMin =
        simd::min(meshTriangle.vertices[meshTriangle.indices.x], meshTriangle.vertices[meshTriangle.indices.y]);
    simd::vec3f boxMax =
        simd::max(meshTriangle.vertices[meshTriangle.indices.x], meshTriangle.vertices[meshTriangle.indices.y]);

    return CoAxisAlignedBoundingBox{
        .min = simd::min(boxMin, meshTriangle.vertices[meshTriangle.indices.z]),
        .max = simd::max(boxMax, meshTriangle.vertices[meshTriangle.indices.z]),
    };
}

bool CoMesh::MeshQuadIntersector::operator()(const CoMesh::MeshQuad &meshQuad) {
    return rayPatchIntersection(
        ray,
        meshQuad.vertices[meshQuad.indices.x],
        meshQuad.vertices[meshQuad.indices.y],
        meshQuad.vertices[meshQuad.indices.z],
        meshQuad.vertices[meshQuad.indices.w],
        event
    );
}

CoAxisAlignedBoundingBox CoMesh::MeshQuadBounder::operator()(const CoMesh::MeshQuad &meshQuad) {
    simd::vec3f boxMin1 = simd::min(meshQuad.vertices[meshQuad.indices.x], meshQuad.vertices[meshQuad.indices.y]);
    simd::vec3f boxMax1 = simd::max(meshQuad.vertices[meshQuad.indices.x], meshQuad.vertices[meshQuad.indices.y]);

    simd::vec3f boxMin2 = simd::min(meshQuad.vertices[meshQuad.indices.z], meshQuad.vertices[meshQuad.indices.w]);
    simd::vec3f boxMax2 = simd::max(meshQuad.vertices[meshQuad.indices.z], meshQuad.vertices[meshQuad.indices.w]);

    return CoAxisAlignedBoundingBox{
        .min = simd::min(boxMin1, boxMin2),
        .max = simd::max(boxMax1, boxMax2),
    };
}

std::shared_ptr<CoMesh> CoMesh::create(const CoMesh::CreateFromFileInfo &createInfo) {
    if (!_checkCreateInfo(createInfo)) {
        return nullptr;
    }

    if (createInfo.fileExtension == "obj") {
        const std::optional<CreateFromBuffersInfo> vertexBuffers = _readObjFile(createInfo.fileName);
        if (vertexBuffers) {
            return std::shared_ptr<CoMesh>(new CoMesh(vertexBuffers.value()));
        }
    }

    return nullptr;
}

CoMesh::CoMesh(const CreateFromBuffersInfo &createInfo)
    : _positions{createInfo.positions}, _primitives{createInfo.indices}, _normals{createInfo.normals},
      _topology{createInfo.topology} {
    switch (_topology) {
    case CoPrimitiveTopology::kTriangle :
        _triangles =
            std::unique_ptr<TriangleAccelerator>(new TriangleAccelerator(TriangleAccelerator::CreateWithPrimitivesInfo{
                .primitives = std::span<MeshTriangle>(_primitives.triangles, _primitives.numPrimitives),
            }));
        break;
    case CoPrimitiveTopology::kQuad :
        _quads = std::unique_ptr<QuadAccelerator>(new QuadAccelerator(QuadAccelerator::CreateWithPrimitivesInfo{
            .primitives = std::span<MeshQuad>(_primitives.quads, _primitives.numPrimitives),
        }));
    }
}

CoMesh::~CoMesh() {
    delete[] _positions.vertices;
    if (_topology == CoPrimitiveTopology::kTriangle) {
        delete[] _primitives.triangles;
    } else {
        delete[] _primitives.quads;
    }

    delete[] _normals.vertices;
}

bool CoMesh::intersects(const CoRay &ray, IntersectionEvent &intersectionEvent) {
    if (_topology == CoPrimitiveTopology::kTriangle) {
        return _triangles->IntersectClosest(ray);
    } else {
        return _quads->IntersectClosest(ray);
    }
}

bool CoMesh::_checkCreateInfo(const CoMesh::CreateFromFileInfo &createInfo) {
    return true;
}

std::optional<CoMesh::CreateFromBuffersInfo> CoMesh::_readObjFile(const std::string &fileName) {
    std::ifstream meshFile(fileName);
    if (!meshFile.good()) {
        CoLogError(CoLogMesh) << "Failed to open mesh file " << fileName;
        return std::nullopt;
    }

    const size_t begginningIdx = meshFile.tellg();
    // prescan to get the number of vertices, normals, and indices count
    const std::optional<MeshBuffersSizeInfo> buffersSizeInfo = _scanMeshBuffersSize(meshFile);
    if (!buffersSizeInfo || buffersSizeInfo->numPositions == 0 || buffersSizeInfo->numFaces == 0) {
        return std::nullopt;
    }

    const bool readNormals = buffersSizeInfo->numNormals > 0;
    VertexBuffer positionsBuffer(buffersSizeInfo->numPositions);
    VertexBuffer normalsBuffer(readNormals ? buffersSizeInfo->numNormals : buffersSizeInfo->numPositions);

    const bool isQuadMesh = buffersSizeInfo->topology == CoPrimitiveTopology::kQuad;
    IndexBuffer indicesBuffer(buffersSizeInfo->numFaces, buffersSizeInfo->topology);

    meshFile.clear();
    meshFile.seekg(begginningIdx);

    size_t positionIdx = 0;
    size_t normalIdx = 0;
    size_t faceIdx = 0;

    static constexpr size_t kMaxLineLength = 256;
    std::string objLine(kMaxLineLength, '\0');
    while (!meshFile.eof()) {
        meshFile.getline(objLine.data(), kMaxLineLength);
        if (objLine[0] == '#') {
            // skip comments
            continue;
        }

        // TODO: I think it is better to split the line here (on spaces),
        // then inside any other sub call
        std::stringstream lineParser(objLine);
        std::string lineInfo;
        lineParser >> lineInfo;
        if (lineInfo == "v") { // vertex
            vec3f vertexPos;
            lineParser >> vertexPos.x >> vertexPos.y >> vertexPos.z;
            positionsBuffer.vertices[positionIdx++] = simd::vec3f(vertexPos.x, vertexPos.y, vertexPos.z);
        } else if (lineInfo == "vn") { // vertex normal
            vec3f vertexNormal;
            lineParser >> vertexNormal.x >> vertexNormal.y >> vertexNormal.z;
            normalsBuffer.vertices[normalIdx++] = simd::vec3f(vertexNormal.x, vertexNormal.y, vertexNormal.z);
        } else if (lineInfo == "f") { // face
            std::string indices;
            vec3i parsedIndices[4] = {};
            size_t currentIndex = 0;
            while (!lineParser.eof()) {
                lineParser >> indices;
                parsedIndices[currentIndex++] = _parseIndices(indices);
            }

            assert(currentIndex == (isQuadMesh ? 4 : 3));
            if (isQuadMesh) {
                indicesBuffer.quads[faceIdx++] = {
                    {uint32_t(parsedIndices[0].x),
                     uint32_t(parsedIndices[1].x),
                     uint32_t(parsedIndices[2].x),
                     uint32_t(parsedIndices[3].x)},
                    positionsBuffer.vertices,
                };
            } else {
                indicesBuffer.triangles[faceIdx++] = {
                    {uint32_t(parsedIndices[0].x), uint32_t(parsedIndices[1].x), uint32_t(parsedIndices[2].x)},
                    positionsBuffer.vertices,
                };
            }
        }
    }

    if (!readNormals) {
        // compute per-face normals
    }

    return CreateFromBuffersInfo{
        .positions = std::move(positionsBuffer),
        .indices = std::move(indicesBuffer),
        .normals = std::move(normalsBuffer),
        .topology = buffersSizeInfo->topology,
    };
}

std::optional<CoMesh::MeshBuffersSizeInfo> CoMesh::_scanMeshBuffersSize(std::ifstream &meshFile) {
    static constexpr size_t kMaxLineLength = 256;
    std::string objLine(kMaxLineLength, '\0');

    MeshBuffersSizeInfo buffersSizeInfo{
        .numPositions = 0,
        .numNormals = 0,
        .numFaces = 0,
        .topology = CoPrimitiveTopology::kTriangle,
    };

    while (!meshFile.eof()) {
        meshFile.getline(objLine.data(), kMaxLineLength);
        if (objLine[0] == '#') {
            // skip comments
            continue;
        }

        std::stringstream lineParser(objLine);
        std::string lineInfo;
        lineParser >> lineInfo;
        if (lineInfo == "v") {         // vertex
            ++buffersSizeInfo.numPositions;
        } else if (lineInfo == "vn") { // vertex normal
            ++buffersSizeInfo.numNormals;
        } else if (lineInfo == "f") {  // face
            std::string indices;
            size_t numVerticesInFace = 0;
            while (!lineParser.eof()) {
                lineParser >> indices;
                ++numVerticesInFace;
            }

            if (numVerticesInFace != 3 && numVerticesInFace != 4) {
                CoLogError(CoLogMesh) << "Unsupported mesh topology";
            }

            buffersSizeInfo.topology =
                (numVerticesInFace == 3) ? CoPrimitiveTopology::kTriangle : CoPrimitiveTopology::kQuad;

            ++buffersSizeInfo.numFaces;
        }
    }

    return buffersSizeInfo;
}

vec3i CoMesh::_parseIndices(const std::string &faceString) {
    vec3i indices = {-1, -1, -1};
    size_t index = 0;
    size_t endIndex = faceString.find_first_of('/', index);
    size_t length = endIndex - index;
    if (endIndex == std::string::npos || length == 0) {
        return indices;
    }

    const std::string posIndex = faceString.substr(index, length);
    indices.x = std::stoi(posIndex) - 1;

    index = endIndex + 1;
    endIndex = faceString.find_first_of('/', index);
    length = endIndex - index;
    if (endIndex != std::string::npos && length != 0) {
        const std::string uvIndex = faceString.substr(index, length);
        indices.y = std::stoi(uvIndex) - 1;
        index = endIndex + 1;
    }

    index = endIndex + 1;
    const size_t stringSize = std::strlen(faceString.c_str());
    endIndex = stringSize - 1;
    length = endIndex - index + 1;
    if (index < stringSize && length != 0) {
        const std::string normalIndex = faceString.substr(index, length);
        indices.z = std::stoi(normalIndex) - 1;
    }

    return indices;
}

} // namespace
  // cblt::geom
