#include "obj_utilities.h"

#include "core/logging.h"
#include "core/size_types.h"
#include "core/string_utilities.h"
#include "geometry/mesh.h"
#include "math/math_types.h"

#include <cstring>
#include <fstream>
#include <memory>
#include <string_view>

namespace {

// common tokents
constexpr const char * const kObjTokens = " \n";
constexpr char kCommentToken = '#';

// mesh tokens
constexpr const char *kPositionToken = "v";
constexpr const char *kNormalToken = "vn";

constexpr const char *kFaceToken = "f";

struct MeshBuffersSizeInfo {
    size_t positionCount = 0;
    size_t normalCount = 0;
    size_t triangleCount = 0;
    size_t patchCount = 0;
};

MeshBuffersSizeInfo scanObjFile(std::ifstream &meshFile) {
    MeshBuffersSizeInfo buffersSizeInfo;

    auto parseFirstToken = [](const std::string_view line) -> std::string_view {
        const size_t splitIdx = line.find_first_of(kObjTokens);
        if (splitIdx == std::string_view::npos) {
            return std::string_view();
        };

        return line.substr(0, splitIdx);
    };

    auto computeTokenCount = [](const std::string_view line) -> size_t {
        size_t tokenCount = 0;
        size_t pos = 0;
        while (pos < line.length()) {
            pos = line.find_first_of(kObjTokens, pos);

            ++tokenCount;
            pos = std::min(line.length(), pos) + 1;
        }

        return tokenCount;
    };

    while (!meshFile.eof()) {
        std::string objLine;
        std::getline(meshFile, objLine);
        if (objLine.empty() || objLine[0] == kCommentToken) {
            // skip comments
            continue;
        }

        const std::string_view lineArguments = parseFirstToken(objLine);
        if (lineArguments.empty()) {
            return MeshBuffersSizeInfo{};
        }

        if (lineArguments == kPositionToken) {      // vertex
            ++buffersSizeInfo.positionCount;
        } else if (lineArguments == kNormalToken) { // vertex normal
            ++buffersSizeInfo.normalCount;
        } else if (lineArguments == kFaceToken) {   // face
            const size_t faceVertexCount = computeTokenCount(objLine) - 1;

            if (faceVertexCount == 3) {
                ++buffersSizeInfo.triangleCount;
            } else if (faceVertexCount == 4) {
                ++buffersSizeInfo.patchCount;
            } else {
                CoLogError("Unsupported mesh topology");
                return MeshBuffersSizeInfo{};
            }
        }
    }

    return buffersSizeInfo;
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
    const MeshBuffersSizeInfo bufferSizeInfo = scanObjFile(meshFile);
    if (!bufferSizeInfo.positionCount || (bufferSizeInfo.triangleCount + bufferSizeInfo.patchCount) == 0) {
        CoLogError("Failed to find valid vertices in mesh");
        return nullptr;
    }

    const MeshBuffersSizeInfo &meshInfo = bufferSizeInfo;

    const geom::CoMesh::VertexAttributeBuffer<simd::vec3f> positions{
        .vertices = std::make_shared<simd::vec3f[]>(meshInfo.positionCount),
        .vertexCount = meshInfo.positionCount,
        .triangleIndices = meshInfo.triangleCount > 0 ? std::make_shared<vec3u[]>(meshInfo.triangleCount) : 0,
        .triangleCount = meshInfo.triangleCount,
        .patchIndices = meshInfo.patchCount > 0 ? std::make_shared<vec4u[]>(meshInfo.patchCount) : 0,
        .patchCount = meshInfo.patchCount,
    };

    meshFile.clear();
    meshFile.seekg(begginningIdx);

    size_t positionIdx = 0;
    size_t triangleIdx = 0;
    size_t patchIdx = 0;

    while (!meshFile.eof()) {
        std::string objLine;
        std::getline(meshFile, objLine);

        if (objLine.empty() || objLine[0] == kCommentToken) {
            // skip comments
            continue;
        }

        const std::vector<std::string> tokens = core::split(objLine, ' ');
        if (tokens[0] == kPositionToken) { // vertex
            if (tokens.size() != 4) {
                CoLogError("Parsing Error: invalid string '%s' for position type", objLine.c_str());
                return nullptr;
            }
            positions.vertices[positionIdx++] = simd::vec3f{
                std::stof(tokens[1]),
                std::stof(tokens[2]),
                std::stof(tokens[3]),
            };
        } else if (tokens[0] == kNormalToken) { // vertex normal
            // TODO: need this?
        } else if (tokens[0] == kFaceToken) { // face
            const size_t indexCount = tokens.size() - 1;
            if (indexCount == 3) {
                positions.triangleIndices[triangleIdx++] = {
                    .x = uint32_t(std::stoi(tokens[1])) - 1,
                    .y = uint32_t(std::stoi(tokens[2])) - 1,
                    .z = uint32_t(std::stoi(tokens[3])) - 1,
                };
            } else if (indexCount == 4) {
                positions.patchIndices[patchIdx++] = {
                    .x = uint32_t(std::stoi(tokens[1])) - 1,
                    .y = uint32_t(std::stoi(tokens[2])) - 1,
                    .z = uint32_t(std::stoi(tokens[3])) - 1,
                    .w = uint32_t(std::stoi(tokens[4])) - 1,
                };
            } else {
                assert(false && "Should never be able to process face without 3 or 4 elements");
            }
        }
    }

    return geom::CoMesh::create({
        .positions = positions,
    });
}

} // namespace cblt::render::utils
