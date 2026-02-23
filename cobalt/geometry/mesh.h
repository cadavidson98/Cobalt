#ifndef COBALT_GEOM_MESH_H
#define COBALT_GEOM_MESH_H

#include "bounding_box.h"
#include "bounding_volume.h"
#include "bounding_volume_mesh_storage.h"
#include "bounding_volume_types.h"

#include "core/size_types.h"
#include "core/vertex_buffer.h"
#include "math/math_types.h"

#include <memory>

namespace cobalt::geom {

struct Ray;
struct IntersectionEvent;

class Mesh {
public:
    struct CreateInfo {
        core::VertexAttributeBuffer<simd::vec3f> positions;
    };

    static std::shared_ptr<Mesh> create(const CreateInfo &createInfo);

    AxisAlignedBoundingBox bounds() const;

    geom::IntersectionResult intersects(const Ray &ray) const;

private:
    using MeshAccelerator = BoundingVolume<MeshStorage>;

    MeshAccelerator _accelerator;
    AxisAlignedBoundingBox _bounds;

    Mesh() = delete;
    Mesh(
        std::shared_ptr<MeshStorage> meshStorage,
        std::span<const MortonPrimitive> meshPrimitives,
        AxisAlignedBoundingBox bounds
    );
};

} // namespace cobalt::geom

#endif // COBALT_GEOM_MESH_H
