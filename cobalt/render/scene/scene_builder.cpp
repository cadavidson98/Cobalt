#include "scene_builder.h"

#include "logging.h"
#include "simd/simd_vec3.h"
#include "vec3.h"
#include "vec4.h"

#include <fstream>
#include <sstream>

CBLT_DEFINE_LOG(CoLogSceneBuilder);

namespace cblt::render {

std::optional<CoSceneBuilder::MeshBuffers> CoSceneBuilder::_readObjFile(const std::string &fileName) {
    std::ifstream meshFile(fileName);
    if (!meshFile.good()) {
        CoLogError(CoLogSceneBuilder) << "Failed to open mesh file " << fileName;
        return std::nullopt;
    }

    const size_t begginningIdx = meshFile.tellg();
    // prescan to get the number of vertices, normals, and indices count
    const std::optional<MeshBuffersSizeInfo> buffersSizeInfo = _scanMeshBuffersSize(meshFile);
    if (!buffersSizeInfo || buffersSizeInfo->numPositions == 0 || buffersSizeInfo->numFaces == 0) {
        return std::nullopt;
    }

    const bool readNormals = buffersSizeInfo->numNormals > 0;
    simd::vec3f *positionsBuffer = new simd::vec3f[buffersSizeInfo->numPositions];

    const bool isQuadMesh = buffersSizeInfo->topology == geom::CoPrimitiveTopology::kQuad;
    vec4u *indicesBuffer = new vec4u[buffersSizeInfo->numFaces];

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
            positionsBuffer[positionIdx++] = simd::vec3f(vertexPos.x, vertexPos.y, vertexPos.z);
        } else if (lineInfo == "vn") { // vertex normal
            vec3f vertexNormal;
            lineParser >> vertexNormal.x >> vertexNormal.y >> vertexNormal.z;
        } else if (lineInfo == "f") {  // face
            std::string indices;
            vec3i parsedIndices[4] = {};
            size_t currentIndex = 0;
            while (!lineParser.eof()) {
                lineParser >> indices;
                parsedIndices[currentIndex++] = _parseIndices(indices);
            }

            assert(currentIndex == (isQuadMesh ? 4 : 3));
            indicesBuffer[faceIdx++] = vec4u{
                uint32_t(parsedIndices[0].x),
                uint32_t(parsedIndices[1].x),
                uint32_t(parsedIndices[2].x),
                uint32_t(parsedIndices[3].x),
            };
        }
    }

    if (!readNormals) {
        // compute per-face normals
    }

    return MeshBuffers{
        .positions = positionsBuffer,
        .numPositions = buffersSizeInfo->numPositions,
        .indices = indicesBuffer,
        .numIndices = buffersSizeInfo->numFaces,
        .topology = buffersSizeInfo->topology,
    };
}

std::optional<CoSceneBuilder::MeshBuffersSizeInfo> CoSceneBuilder::_scanMeshBuffersSize(std::ifstream &meshFile) {
    static constexpr size_t kMaxLineLength = 256;
    std::string objLine(kMaxLineLength, '\0');

    MeshBuffersSizeInfo buffersSizeInfo{
        .numPositions = 0,
        .numNormals = 0,
        .numFaces = 0,
        .topology = geom::CoPrimitiveTopology::kTriangle,
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
                CoLogError(CoLogSceneBuilder) << "Unsupported mesh topology";
            }

            buffersSizeInfo.topology =
                (numVerticesInFace == 3) ? geom::CoPrimitiveTopology::kTriangle : geom::CoPrimitiveTopology::kQuad;

            ++buffersSizeInfo.numFaces;
        }
    }

    return buffersSizeInfo;
}

vec3i CoSceneBuilder::_parseIndices(const std::string &faceString) {
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

} // namespace cblt::render
