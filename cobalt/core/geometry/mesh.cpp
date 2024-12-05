#include "mesh.h"

#include "logging.h"
#include "vec3.h"

#include <fstream>
#include <sstream>

CBLT_DEFINE_LOG(CoLogMesh);

namespace cblt::geom {

CoMesh::VertexBuffer::VertexBuffer(size_t _numVertices, size_t _numIndices)
    : numVertices{_numVertices}, numIndices{_numIndices}, vertices{nullptr}, indices{nullptr} {
    vertices = new simd::vec3f[numVertices];
    indices = new uint32_t[numIndices];
}

std::shared_ptr<CoMesh> CoMesh::create(const CoMesh::CreateFromFileInfo &createInfo) {
    if (!_checkCreateInfo(createInfo)) {
        return nullptr;
    }

    if (createInfo.fileExtension ==
        "ob"
        "j") {
        const std::optional<CreateFromBuffersInfo> vertexBuffers = _readObjFile(createInfo.fileName);
        if (vertexBuffers) {
            return std::shared_ptr<CoMesh>(new CoMesh(vertexBuffers.value()));
        }
    }

    return nullptr;
}

CoMesh::CoMesh(const CreateFromBuffersInfo &createInfo)
    : _positions{createInfo.positions}, _normals{createInfo.normals}, _topology{createInfo.topology} {
}

CoMesh::~CoMesh() {
    delete[] _positions.vertices;
    delete[] _positions.indices;
    delete[] _normals.vertices;
    delete[] _normals.indices;
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
    const std::optional<MeshBuffersSizeInfo> buffersSizeInfo = _scanVertexBuffersSize(meshFile);
    if (!buffersSizeInfo || buffersSizeInfo->numPositions == 0 || buffersSizeInfo->numIndices == 0) {
        return std::nullopt;
    }

    const bool readNormals = buffersSizeInfo->numNormals > 0;
    VertexBuffer positionsBuffer(buffersSizeInfo->numPositions, buffersSizeInfo->numIndices);
    VertexBuffer normalsBuffer(
        readNormals ? buffersSizeInfo->numNormals : buffersSizeInfo->numPositions,
        buffersSizeInfo->numIndices
    );

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
            while (!lineParser.eof()) {
                lineParser >> indices;
                const vec3i vertexIndices = _parseIndices(indices);
                positionsBuffer.indices[faceIdx] = vertexIndices.x;
                if (readNormals) {
                    normalsBuffer.indices[faceIdx] = vertexIndices.z;
                }
                ++faceIdx;
            }
        }
    }

    if (!readNormals) {
        // compute per-face normals
    }

    return CreateFromBuffersInfo{
        .positions = std::move(positionsBuffer),
        .normals = std::move(normalsBuffer),
        .topology = buffersSizeInfo->topology,
    };
}

std::optional<CoMesh::MeshBuffersSizeInfo> CoMesh::_scanVertexBuffersSize(std::ifstream &meshFile) {
    static constexpr size_t kMaxLineLength = 256;
    std::string objLine(kMaxLineLength, '\0');

    MeshBuffersSizeInfo buffersSizeInfo{
        .numPositions = 0,
        .numNormals = 0,
        .numIndices = 0,
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

            buffersSizeInfo.numIndices += numVerticesInFace;
        }
    }

    buffersSizeInfo.topology =
        (buffersSizeInfo.numIndices % 3 == 0) ? CoPrimitiveTopology::kTriangle : CoPrimitiveTopology::kQuad;
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
    endIndex = faceString.size() - 1;
    length = endIndex - index + 1;
    if (length != 0) {
        const std::string normalIndex = faceString.substr(index, length);
        indices.z = std::stoi(normalIndex) - 1;
    }

    return indices;
}

} // namespace
  // cblt::geom
