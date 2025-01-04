#include "scene_builder.h"

#include "logging.h"
#include "simd/simd_vec3.h"
#include "vec3.h"
#include "vec4.h"

#include <cstring>
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

    vec4u *indicesBuffer = new vec4u[buffersSizeInfo->numFaces];

    meshFile.clear();
    meshFile.seekg(begginningIdx);

    size_t positionIdx = 0;
    size_t normalIdx = 0;
    size_t faceIdx = 0;

    const char *kObjTokens = " \n";
    static constexpr size_t kMaxLineLength = 256;
    char objLine[kMaxLineLength] = {'\0'};

    auto stringToVec3 = [kObjTokens]() {
        vec3f value;
        char *floatString = nullptr;
        value.x = std::stof(std::strtok(nullptr, kObjTokens));
        value.y = std::stof(std::strtok(nullptr, kObjTokens));
        value.z = std::stof(std::strtok(nullptr, kObjTokens));
        return simd::vec3f(value.x, value.y, value.z);
    };

    while (!meshFile.eof()) {
        std::memset(objLine, '\0', kMaxLineLength);
        meshFile.getline(objLine, kMaxLineLength);
        if (objLine[0] == '#' || std::strlen(objLine) == 0) {
            // skip comments
            continue;
        }

        // TODO: I think it is better to split the line here (on spaces),
        // then inside any other sub call
        char *lineInfo = std::strtok(objLine, kObjTokens);
        if (std::strncmp(lineInfo, "v", kMaxLineLength) == 0) { // vertex
            positionsBuffer[positionIdx++] = stringToVec3();
        } else if (std::strncmp(lineInfo, "vn", kMaxLineLength) == 0) { // vertex normal
            // TODO: need this?
        } else if (std::strncmp(lineInfo, "f", kMaxLineLength) == 0) {  // face
            char *indicesString = nullptr;
            vec3i parsedIndices[4] = {{-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1}};
            size_t currentIndex = 0;
            while ((indicesString = std::strtok(nullptr, kObjTokens)) != nullptr) {
                std::string indices(indicesString);
                parsedIndices[currentIndex++] = _parseIndices(indices);
            }

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
    };
}

std::optional<CoSceneBuilder::MeshBuffersSizeInfo> CoSceneBuilder::_scanMeshBuffersSize(std::ifstream &meshFile) {
    static constexpr size_t kMaxLineLength = 256;
    char objLine[kMaxLineLength] = {'\0'};

    size_t indexCount = 0;

    MeshBuffersSizeInfo buffersSizeInfo{
        .numPositions = 0,
        .numNormals = 0,
        .numFaces = 0,
    };

    while (!meshFile.eof()) {
        std::memset(objLine, '\0', kMaxLineLength);
        meshFile.getline(objLine, kMaxLineLength);
        if (objLine[0] == '#' || std::strlen(objLine) == 0) {
            // skip comments
            continue;
        }

        const char *objTokens = " \n";
        char *lineArguments = std::strtok(objLine, objTokens);
        if (!lineArguments) {
            return std::nullopt;
        }
        if (std::strncmp(lineArguments, "v", kMaxLineLength) == 0) {         // vertex
            ++buffersSizeInfo.numPositions;
        } else if (std::strncmp(lineArguments, "vn", kMaxLineLength) == 0) { // vertex normal
            ++buffersSizeInfo.numNormals;
        } else if (std::strncmp(lineArguments, "f", kMaxLineLength) == 0) {  // face
            size_t numVerticesInFace = 0;
            while (std::strtok(nullptr, objTokens) != nullptr) {
                ++numVerticesInFace;
            }

            if (indexCount == 0) {
                indexCount = numVerticesInFace;
            }

            if (numVerticesInFace != 3 && numVerticesInFace != 4) {
                CoLogError(CoLogSceneBuilder) << "Unsupported mesh topology";
                return std::nullopt;
            }

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
