#ifndef CBLT_GEOM_MESH_H
#define CBLT_GEOM_MESH_H

#include "bounding_volume_crtp.h"
#include "bounding_volume_mesh_storage.h"
#include "bounding_volume_types.h"

#include "core/size_types.h"
#include "geometry/bounding_box.h"
#include "math/math_types.h"

#include <memory>

namespace cblt::geom {

struct CoRay;
struct IntersectionEvent;

class CoMesh {
public:
    enum class VertexAttribute {
        kPosition,
        kNormal,
        kUV,
    };

    template<typename T>
    struct VertexAttributeBuffer {
        std::shared_ptr<T[]> vertices;
        size_t vertexCount;
        std::shared_ptr<vec3u[]> triangleIndices;
        size_t triangleCount;
        std::shared_ptr<vec4u[]> patchIndices;
        size_t patchCount;
    };

    struct CreateInfo {
        VertexAttributeBuffer<simd::vec3f> positions;
    };

    static std::shared_ptr<CoMesh> create(const CreateInfo &createInfo);

    CoAxisAlignedBoundingBox bounds() const;

    geom::crtp::IntersectionResult intersects(const CoRay &ray) const;

private:
    using MeshAccelerator = crtp::CoBoundingVolume<crtp::CoMeshStorage>;

    CoAxisAlignedBoundingBox _bounds;

    VertexAttributeBuffer<simd::vec3f> _positions;
    std::unique_ptr<MeshAccelerator> _accelerator;

    CoMesh() = delete;
    CoMesh(const CreateInfo &createInfo);
};

} // namespace cblt::geom

#endif // CBLT_GEOM_MESH_H
