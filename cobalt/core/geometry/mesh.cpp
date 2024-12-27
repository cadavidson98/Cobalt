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

CoMesh::TriangleStorage::TriangleStorage(simd::vec3f *positions, vec4u *indices)
    : _positions{positions}, _indices{indices} {
}

CoMesh::TriangleStorage::~TriangleStorage() {
    delete[] _positions;
    delete[] _indices;
}

size_t CoMesh::TriangleStorage::NumPrimitives() const {
    return numIndices;
}

CoAxisAlignedBoundingBox CoMesh::TriangleStorage::PrimitiveBounds(size_t startIdx, size_t endIdx) const {
    static constexpr float minFloat = std::numeric_limits<float>::lowest();
    static constexpr float maxFloat = std::numeric_limits<float>::max();
    CoAxisAlignedBoundingBox regionBounds = {
        .min = simd::vec3f(maxFloat, maxFloat, maxFloat),
        .max = simd::vec3f(minFloat, minFloat, minFloat),
    };

    std::vector<CoAxisAlignedBoundingBox> quadBounds = _ComputePrimitiveBounds(startIdx, endIdx);

    for (size_t idx = startIdx; idx < endIdx; ++idx) {
        regionBounds.min = simd::min(regionBounds.min, quadBounds[idx].min);
        regionBounds.max = simd::max(regionBounds.max, quadBounds[idx].max);
    }

    return regionBounds;
}

size_t CoMesh::TriangleStorage::Reorder(
    size_t startIdx,
    size_t endIdx,
    std::function<bool(const CoAxisAlignedBoundingBox &)> comparator
) {
    std::vector<CoAxisAlignedBoundingBox> bounds = _ComputePrimitiveBounds(startIdx, endIdx);
    size_t splitIdx = endIdx;
    // partition
    for (size_t idx = startIdx; idx < endIdx;) {
        if (comparator(bounds[idx])) {
            ++idx;
        } else {
            --splitIdx;
            std::swap(_indices[idx], _indices[splitIdx]);
        }
    }

    return splitIdx;
}

bool CoMesh::TriangleStorage::PrimitivesIntersect(
    const CoRay &ray,
    size_t startIdx,
    size_t endIdx,
    IntersectionEvent &event
) const {
    for (size_t idx = startIdx; idx < endIdx; ++idx) {
        const vec4u &triangleIndex = _indices[startIdx];
        if (rayTriangleIntersection(
                ray,
                _positions[triangleIndex.x],
                _positions[triangleIndex.y],
                _positions[triangleIndex.z],
                event
            )) {
            return true;
        }
    }

    return false;
}

std::vector<CoAxisAlignedBoundingBox>
CoMesh::TriangleStorage::_ComputePrimitiveBounds(size_t startIdx, size_t endIdx) const {
    std::vector<CoAxisAlignedBoundingBox> bounds;
    bounds.reserve(endIdx - startIdx + 1);
    for (size_t idx = startIdx; idx < endIdx; ++idx) {
        const vec4u &triangleIndex = _indices[idx];
        const simd::vec3f boxMin12 = simd::min(_positions[triangleIndex.x], _positions[triangleIndex.y]);
        const simd::vec3f boxMin34 = simd::min(_positions[triangleIndex.z], _positions[triangleIndex.w]);
        const simd::vec3f boxMin = simd::min(boxMin12, boxMin34);

        const simd::vec3f boxMax12 = simd::max(_positions[triangleIndex.x], _positions[triangleIndex.y]);
        const simd::vec3f boxMax34 = simd::max(_positions[triangleIndex.z], _positions[triangleIndex.w]);
        const simd::vec3f boxMax = simd::max(boxMin12, boxMin34);

        bounds.push_back({
            .min = boxMin,
            .max = boxMax,
        });
    }

    return bounds;
}

CoMesh::QuadStorage::QuadStorage(simd::vec3f *positions, vec4u *indices, size_t numIndices)
    : _positions{positions}, _indices{indices}, _numIndices{numIndices} {
    _bounds = _ComputePrimitiveBounds(0, numIndices);
}

CoMesh::QuadStorage::~QuadStorage() {
    delete[] _positions;
    delete[] _indices;
}

size_t CoMesh::QuadStorage::NumPrimitives() const {
    return _numIndices;
}

CoAxisAlignedBoundingBox CoMesh::QuadStorage::PrimitiveBounds(size_t startIdx, size_t endIdx) const {
    static constexpr float minFloat = std::numeric_limits<float>::lowest();
    static constexpr float maxFloat = std::numeric_limits<float>::max();
    CoAxisAlignedBoundingBox regionBounds = {
        .min = simd::vec3f(maxFloat, maxFloat, maxFloat),
        .max = simd::vec3f(minFloat, minFloat, minFloat),
    };

    for (size_t idx = startIdx; idx < endIdx; ++idx) {
        regionBounds.min = simd::min(regionBounds.min, _bounds[idx].min);
        regionBounds.max = simd::max(regionBounds.max, _bounds[idx].max);
    }

    return regionBounds;
}

size_t CoMesh::QuadStorage::Reorder(
    size_t startIdx,
    size_t endIdx,
    std::function<bool(const CoAxisAlignedBoundingBox &)> comparator
) {
    size_t splitIdx = endIdx - 1;
    // partition
    for (size_t idx = startIdx; idx < splitIdx;) {
        if (comparator(_bounds[idx])) {
            ++idx;
        } else {
            --splitIdx;
            std::swap(_indices[idx], _indices[splitIdx]);
            std::swap(_bounds[idx], _bounds[splitIdx]);
        }
    }

    return splitIdx;
}

bool CoMesh::QuadStorage::PrimitivesIntersect(
    const CoRay &ray,
    size_t startIdx,
    size_t endIdx,
    IntersectionEvent &event
) const {
    bool hitPatch = false;
    for (size_t idx = startIdx; idx < endIdx; ++idx) {
        const vec4u &patchIndex = _indices[idx];
        hitPatch = rayPatchIntersection(
                       ray,
                       _positions[patchIndex.x],
                       _positions[patchIndex.y],
                       _positions[patchIndex.z],
                       _positions[patchIndex.w],
                       event
                   ) ||
                   hitPatch;
    }

    return hitPatch;
}

std::vector<CoAxisAlignedBoundingBox>
CoMesh::QuadStorage::_ComputePrimitiveBounds(size_t startIdx, size_t endIdx) const {
    std::vector<CoAxisAlignedBoundingBox> bounds;
    bounds.reserve(endIdx - startIdx + 1);
    for (size_t idx = startIdx; idx < endIdx; ++idx) {
        const vec4u &triangleIndex = _indices[idx];
        const simd::vec3f boxMin12 = simd::min(_positions[triangleIndex.x], _positions[triangleIndex.y]);
        const simd::vec3f boxMin34 = simd::min(_positions[triangleIndex.z], _positions[triangleIndex.w]);
        const simd::vec3f boxMin = simd::min(boxMin12, boxMin34);

        const simd::vec3f boxMax12 = simd::max(_positions[triangleIndex.x], _positions[triangleIndex.y]);
        const simd::vec3f boxMax34 = simd::max(_positions[triangleIndex.z], _positions[triangleIndex.w]);
        const simd::vec3f boxMax = simd::max(boxMin12, boxMin34);

        bounds.push_back({
            .min = boxMin,
            .max = boxMax,
        });
    }

    return bounds;
}

/// ----------------------------------- CoMesh -----------------------------------

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

CoMesh::CoMesh(const CreateFromBuffersInfo &createInfo): _topology{createInfo.topology} {
    switch (_topology) {
    case CoPrimitiveTopology::kTriangle :
        _triangles = std::shared_ptr<TriangleStorage>(new TriangleStorage(createInfo.positions, createInfo.indices));
        _triangleAccelerator =
            std::unique_ptr<TriangleAccelerator>(new TriangleAccelerator(TriangleAccelerator::CreateWithPrimitivesInfo{
                .primitives = _triangles,
            }));
        break;
    case CoPrimitiveTopology::kQuad :
        _quads = std::shared_ptr<QuadStorage>(
            new QuadStorage(createInfo.positions, createInfo.indices, createInfo.numIndices)
        );
        _quadAccelerator =
            std::unique_ptr<QuadAccelerator>(new QuadAccelerator(QuadAccelerator::CreateWithPrimitivesInfo{
                .primitives = _quads,
            }));
    }
}

CoMesh::~CoMesh() {
}

bool CoMesh::intersects(const CoRay &ray, IntersectionEvent &intersectionEvent) {
    if (_topology == CoPrimitiveTopology::kTriangle) {
        return _triangleAccelerator->IntersectClosest(ray, intersectionEvent);
    } else {
        // return _quads->PrimitivesIntersect(ray, 0, _quads->NumPrimitives(), intersectionEvent);
        return _quadAccelerator->IntersectClosest(ray, intersectionEvent);
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
    simd::vec3f *positionsBuffer = new simd::vec3f[buffersSizeInfo->numPositions];

    const bool isQuadMesh = buffersSizeInfo->topology == CoPrimitiveTopology::kQuad;
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

    return CreateFromBuffersInfo{
        .positions = positionsBuffer,
        .numVertices = buffersSizeInfo->numPositions,
        .indices = indicesBuffer,
        .numIndices = buffersSizeInfo->numFaces,
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
