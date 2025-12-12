#ifndef CBLT_GEOM_MESH_H
#define CBLT_GEOM_MESH_H

#include "bounding_box.h"
#include "bounding_volume.h"
#include "bounding_volume_mesh_storage.h"
#include "bounding_volume_types.h"

#include "core/size_types.h"
#include "core/vertex_buffer.h"
#include "math/math_types.h"

#include <memory>

namespace cblt::geom {

struct CoRay;
struct IntersectionEvent;

class CoMesh {
public:
    struct CreateInfo {
        core::VertexAttributeBuffer<simd::vec3f> positions;
    };

    static std::shared_ptr<CoMesh> create(const CreateInfo &createInfo);

    CoAxisAlignedBoundingBox bounds() const;

    geom::IntersectionResult intersects(const CoRay &ray) const;

private:
    using MeshAccelerator = CoBoundingVolume<CoMeshStorage>;

    MeshAccelerator _accelerator;
    CoAxisAlignedBoundingBox _bounds;

    CoMesh() = delete;
    CoMesh(
        std::shared_ptr<CoMeshStorage> meshStorage,
        std::span<const MortonPrimitive> meshPrimitives,
        CoAxisAlignedBoundingBox bounds
    );
};

} // namespace cblt::geom

#endif // CBLT_GEOM_MESH_H
