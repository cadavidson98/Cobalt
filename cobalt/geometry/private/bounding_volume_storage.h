#ifndef CBLT_GEOM_BOUNDING_VOLUME_STORAGE_H
#define CBLT_GEOM_BOUNDING_VOLUME_STORAGE_H

#include "bounding_volume_types.h"
#include <filesystem>

namespace cblt::geom::crtp {

class CoBoundingVolumeStorage {
public:

private:

};

enum Format {
    kFormatUInt3,
    kFormatUInt4,
};

struct VertexBuffer {
    vec3f *positions;
    size_t numPts;

    uint32_t *indices;
    size_t numIndices;

    Format format;
};

struct CStyleStorage {
    Primitive *primitives;
    size_t numPrimitives;

    VertexBuffer *triangles;
    VertexBuffer *patches;

    struct Sphere *spheres;
    size_t numSpheres;

    struct Box *boxes;
    size_t numBoxes;

    struct Mesh *meshes;
    size_t numMeshes;
};

void reorder(CStyleStorage *storage) {
    size_t sphereIdx = 0;
    size_t boxIdx = 0;
    size_t meshIdx = 0;

    CStyleStorage *copy = clone(storage);

    for (size_t primitiveIdx = 0; primitiveIdx < copy->numPrimitives; ++primitiveIdx) {
        Primitive primitive = copy->primitives[primitiveIdx];
        switch (primitive.type) {
        case PrimitiveType::kSphere:
            storage->spheres[(sphereIdx++)] = copy->spheres[primitive.index];
            continue;
        case PrimitiveType::kBox:
            storage->boxes[(boxIdx++)] = copy->boxes[primitive.index];
            continue;
        case PrimitiveType::kMesh:
            storage->meshes[(meshIdx++)] = copy->meshes[primitive.index];
        default: [[fallthrough]];
        }
    }
}

}  // namespace cblt::geom

#endif  // CBLT_GEOM_BOUNDING_VOLUME_STORAGE_H