#include "obj_reader.h"

#include "core/logging.h"
#include "core/size_types.h"
#include "core/string_utilities.h"
#include "math/math_types.h"
#include "math/vec2.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <memory>
#include <string>

namespace cobalt::io::obj {

namespace {

// common tokents
constexpr const char * const kObjTokens = " \n";
constexpr char kCommentToken = '#';

// mesh tokens
constexpr const char *kPositionToken = "v";
constexpr const char *kTextureToken = "vt";
constexpr const char *kNormalToken = "vn";

constexpr const char *kFaceToken = "f";

struct MeshBuffersSize {
    size_t positionCount = 0;
    size_t uvCount = 0;
    size_t normalCount = 0;
    size_t triangleCount = 0;
    size_t patchCount = 0;
};

MeshBuffersSize scan(std::ifstream &meshFile) {
    MeshBuffersSize buffersSizeInfo;

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
            return MeshBuffersSize{};
        }

        if (lineArguments == kPositionToken) {       // vertex
            ++buffersSizeInfo.positionCount;
        } else if (lineArguments == kNormalToken) {  // vertex normal
            ++buffersSizeInfo.normalCount;
        } else if (lineArguments == kTextureToken) { // vertex texture coordinate
            ++buffersSizeInfo.uvCount;
        } else if (lineArguments == kFaceToken) {    // face
            const size_t faceVertexCount = computeTokenCount(objLine) - 1;

            if (faceVertexCount == 3) {
                ++buffersSizeInfo.triangleCount;
            } else if (faceVertexCount == 4) {
                ++buffersSizeInfo.patchCount;
            } else {
                CoLogError("Unsupported mesh topology");
                return MeshBuffersSize{};
            }
        }
    }

    return buffersSizeInfo;
}

struct VertexIndices {
    std::string position;
    std::string uv;
    std::string normal;
};

VertexIndices splitFaceIndices(const std::string_view vertexIndices) {
    const size_t positionIdx = vertexIndices.find_first_of('/');
    const size_t uvIdx = vertexIndices.find_last_of('/');

    assert(uvIdx > positionIdx);

    return {
        .position = std::string(vertexIndices.substr(0, positionIdx)),
        .uv = std::string(vertexIndices.substr(positionIdx + 1, uvIdx - (positionIdx + 1))),
        .normal = std::string(vertexIndices.substr(uvIdx + 1)),
    };
}

} // anonymous namespace

std::optional<Mesh> read(const std::string_view fileName) {
    std::ifstream meshFile(fileName.data());
    if (!meshFile.good()) {
        CoLogError("Failed to open mesh file %s", fileName.data());
        return std::nullopt;
    }

    const size_t begginningIdx = meshFile.tellg();
    // prescan to get the number of vertices, normals, and indices count
    const MeshBuffersSize meshInfo = scan(meshFile);
    if (!meshInfo.positionCount || (meshInfo.triangleCount + meshInfo.patchCount) == 0) {
        CoLogError("Failed to find valid vertices in mesh");
        return std::nullopt;
    }

    const core::VertexAttributeBuffer<simd::vec3f> positions{
        .vertices = std::make_shared<simd::vec3f[]>(meshInfo.positionCount),
        .vertexCount = meshInfo.positionCount,
        .triangleIndices = meshInfo.triangleCount > 0 ? std::make_shared<vec3u[]>(meshInfo.triangleCount) : nullptr,
        .triangleCount = meshInfo.triangleCount,
        .patchIndices = meshInfo.patchCount > 0 ? std::make_shared<vec4u[]>(meshInfo.patchCount) : nullptr,
        .patchCount = meshInfo.patchCount,
    };

    const core::VertexAttributeBuffer<simd::vec3f> normals{
        .vertices = meshInfo.normalCount > 0 ? std::make_shared<simd::vec3f[]>(meshInfo.normalCount) : nullptr,
        .vertexCount = meshInfo.normalCount,
        .triangleIndices = meshInfo.triangleCount > 0 ? std::make_shared<vec3u[]>(meshInfo.triangleCount) : nullptr,
        .triangleCount = meshInfo.triangleCount,
        .patchIndices = meshInfo.patchCount > 0 ? std::make_shared<vec4u[]>(meshInfo.patchCount) : nullptr,
        .patchCount = meshInfo.patchCount,
    };

    const core::VertexAttributeBuffer<vec2f> uvs{
        .vertices = meshInfo.uvCount > 0 ? std::make_shared<vec2f[]>(meshInfo.uvCount) : nullptr,
        .vertexCount = meshInfo.uvCount,
        .triangleIndices = meshInfo.triangleCount > 0 ? std::make_shared<vec3u[]>(meshInfo.triangleCount) : nullptr,
        .triangleCount = meshInfo.triangleCount,
        .patchIndices = meshInfo.patchCount > 0 ? std::make_shared<vec4u[]>(meshInfo.patchCount) : nullptr,
        .patchCount = meshInfo.patchCount,
    };

    meshFile.clear();
    meshFile.seekg(begginningIdx);

    size_t positionIdx = 0;
    size_t uvIdx = 0;
    size_t normalIdx = 0;
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
                return std::nullopt;
            }
            positions.vertices[positionIdx++] = simd::vec3f{
                std::stof(tokens[1]),
                std::stof(tokens[2]),
                std::stof(tokens[3]),
            };
        } else if (tokens[0] == kTextureToken) { // vertex texture coordinate
            if (tokens.size() != 3) {
                CoLogError("Parsing Error: invalid string '%s' for texture coordinate type", objLine.c_str());
                return std::nullopt;
            }
            uvs.vertices[uvIdx++] = vec2f{
                std::stof(tokens[1]),
                std::stof(tokens[2]),
            };
        } else if (tokens[0] == kNormalToken) { // vertex normal
            if (tokens.size() != 4) {
                CoLogError("Parsing Error: invalid string '%s' for normal type", objLine.c_str());
                return std::nullopt;
            }
            normals.vertices[normalIdx++] = simd::vec3f{
                std::stof(tokens[1]),
                std::stof(tokens[2]),
                std::stof(tokens[3]),
            };
        } else if (tokens[0] == kFaceToken) { // face
            const size_t indexCount = tokens.size() - 1;
            std::vector<VertexIndices> indices;
            std::transform(tokens.begin() + 1, tokens.end(), std::back_inserter(indices), splitFaceIndices);

            auto toIdx = [](const std::string_view idxString) -> uint32_t {
                return uint32_t(std::stoi(idxString.data()) - 1);
            };

            if (indexCount == 3) {
                const size_t idx = triangleIdx++;

                positions.triangleIndices[idx] = {
                    .x = toIdx(indices[0].position),
                    .y = toIdx(indices[1].position),
                    .z = toIdx(indices[2].position),
                };

                if (meshInfo.uvCount) {
                    assert(!indices[0].uv.empty() && !indices[1].uv.empty() && !indices[2].uv.empty());
                    uvs.triangleIndices[idx] = {
                        .x = toIdx(indices[0].uv),
                        .y = toIdx(indices[1].uv),
                        .z = toIdx(indices[2].uv),
                    };
                }

                if (meshInfo.normalCount) {
                    assert(!indices[0].normal.empty() && !indices[1].normal.empty() && !indices[2].normal.empty());
                    normals.triangleIndices[idx] = {
                        .x = toIdx(indices[0].normal),
                        .y = toIdx(indices[1].normal),
                        .z = toIdx(indices[2].normal),
                    };
                }
            } else if (indexCount == 4) {
                const size_t idx = patchIdx++;

                assert(indices.size() == 4);

                positions.patchIndices[idx] = {
                    .x = toIdx(indices[0].position),
                    .y = toIdx(indices[1].position),
                    .z = toIdx(indices[2].position),
                    .w = toIdx(indices[3].position),
                };

                if (meshInfo.uvCount) {
                    assert(
                        !indices[0].uv.empty() && !indices[1].uv.empty() && !indices[2].uv.empty() &&
                        !indices[3].uv.empty()
                    );
                    uvs.patchIndices[idx] = {
                        .x = toIdx(indices[0].uv),
                        .y = toIdx(indices[1].uv),
                        .z = toIdx(indices[2].uv),
                        .w = toIdx(indices[3].uv),
                    };
                }

                if (meshInfo.normalCount) {
                    assert(
                        !indices[0].normal.empty() && !indices[1].normal.empty() && !indices[2].normal.empty() &&
                        !indices[3].normal.empty()
                    );
                    normals.patchIndices[idx] = {
                        .x = toIdx(indices[0].normal),
                        .y = toIdx(indices[1].normal),
                        .z = toIdx(indices[2].normal),
                        .w = toIdx(indices[3].normal),
                    };
                }
            } else {
                assert(false && "Should never be able to process face without 3 or 4 elements");
            }
        }
    }

    return Mesh{
        .positions = positions,
        .uvs = uvs,
        .normals = normals,
    };
}

} // namespace cobalt::io::obj
