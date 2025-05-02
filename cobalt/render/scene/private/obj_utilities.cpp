#include "obj_utilities.h"

#include "core/logging.h"
#include "core/size_types.h"
#include "geometry/mesh.h"
#include "math/math_types.h"

#include <cstring>
#include <fstream>
#include <optional>
#include <unordered_set>
#include <vector>

namespace {

// common tokents
const char *kObjTokens = " \n";
const char kCommentToken = '#';

// mesh tokens
const char *kPositionToken = "v";
const char *kTextureCoordinateToken = "vt";
const char *kNormalToken = "vn";

const char *kFaceToken = "f";

struct MeshBuffersSizeInfo {
    size_t numPositions = 0;
    size_t numNormals = 0;
    size_t numFaces = 0;
};

std::optional<MeshBuffersSizeInfo> scanObjFile(std::ifstream &meshFile) {
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
        if (objLine[0] == kCommentToken || std::strlen(objLine) == 0) {
            // skip comments
            continue;
        }

        char *lineArguments = std::strtok(objLine, kObjTokens);
        if (!lineArguments) {
            return std::nullopt;
        }

        if (std::strncmp(lineArguments, kPositionToken, kMaxLineLength) == 0) {      // vertex
            ++buffersSizeInfo.numPositions;
        } else if (std::strncmp(lineArguments, kNormalToken, kMaxLineLength) == 0) { // vertex normal
            ++buffersSizeInfo.numNormals;
        } else if (std::strncmp(lineArguments, kFaceToken, kMaxLineLength) == 0) {   // face
            size_t numVerticesInFace = 0;
            while (std::strtok(nullptr, kObjTokens) != nullptr) {
                ++numVerticesInFace;
            }

            if (indexCount == 0) {
                indexCount = numVerticesInFace;
            }

            if (numVerticesInFace != 3 && numVerticesInFace != 4) {
                CoLogError("Unsupported mesh topology");
                return std::nullopt;
            }

            ++buffersSizeInfo.numFaces;
        }
    }

    return buffersSizeInfo;
}

cblt::vec3i parseIndices(const std::string &faceString) {
    cblt::vec3i indices = {-1, -1, -1};
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

} // anonymous namespace

namespace cblt::render::utils {

std::shared_ptr<geom::CoMesh> readObjFile(const std::string &fileName) {
    std::ifstream meshFile(fileName);
    if (!meshFile.good()) {
        CoLogError("Failed to open mesh file %s", fileName.c_str());
        return nullptr;
    }

    const size_t begginningIdx = meshFile.tellg();
    // prescan to get the number of vertices, normals, and indices count
    std::optional<MeshBuffersSizeInfo> bufferSizeInfo = scanObjFile(meshFile);
    if (!bufferSizeInfo || !bufferSizeInfo->numPositions || !bufferSizeInfo->numFaces) {
        CoLogError("Failed to find valid vertices in mesh");
        return nullptr;
    }

    const MeshBuffersSizeInfo &meshInfo = bufferSizeInfo.value();

    simd::vec3f *positionsBuffer = new simd::vec3f[meshInfo.numPositions];

    vec4u *indicesBuffer = new vec4u[meshInfo.numFaces];

    meshFile.clear();
    meshFile.seekg(begginningIdx);

    size_t positionIdx = 0;
    size_t normalIdx = 0;
    size_t faceIdx = 0;

    static constexpr size_t kMaxLineLength = 256;
    char objLine[kMaxLineLength] = {'\0'};

    auto stringToVec3 = []() {
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
        if (objLine[0] == kCommentToken || std::strlen(objLine) == 0) {
            // skip comments
            continue;
        }

        // TODO: I think it is better to split the line here (on spaces),
        // then inside any other sub call
        char *lineInfo = std::strtok(objLine, kObjTokens);
        if (std::strncmp(lineInfo, kPositionToken, kMaxLineLength) == 0) {      // vertex
            positionsBuffer[positionIdx++] = stringToVec3();
        } else if (std::strncmp(lineInfo, kNormalToken, kMaxLineLength) == 0) { // vertex normal
            // TODO: need this?
        } else if (std::strncmp(lineInfo, kFaceToken, kMaxLineLength) == 0) { // face
            char *indicesString = nullptr;
            vec3i parsedIndices[4] = {
                {-1, -1, -1},
                {-1, -1, -1},
                {-1, -1, -1},
                {-1, -1, -1}
            };
            size_t currentIndex = 0;
            while ((indicesString = std::strtok(nullptr, kObjTokens)) != nullptr) {
                std::string indices(indicesString);
                parsedIndices[currentIndex++] = parseIndices(indices);
            }

            indicesBuffer[faceIdx++] = vec4u{
                uint32_t(parsedIndices[0].x),
                uint32_t(parsedIndices[1].x),
                uint32_t(parsedIndices[2].x),
                uint32_t(parsedIndices[3].x),
            };
        }
    }

    return geom::CoMesh::create({
        .positions = positionsBuffer,
        .numVertices = meshInfo.numPositions,
        .indices = indicesBuffer,
        .numIndices = meshInfo.numFaces,
    });
}

} // namespace cblt::render::utils
