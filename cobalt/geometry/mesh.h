#ifndef CBLT_CORE_MESH_H
#define CBLT_CORE_MESH_H

#include "bounding_volume.h"

#include "core/size_types.h"
#include "math/simd/simd_vec3.h"
#include "math/vec3.h"
#include "math/vec4.h"

#include <functional>
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

    struct Vertex {
        vec3f position;
        vec3f normal;
        vec2f textureCoords;
    };

    struct CreateInfo {
        simd::vec3f *positions;
        size_t numVertices;
        vec4u *indices;
        size_t numIndices;
    };

    static std::shared_ptr<CoMesh> create(const CreateInfo &createInfo);

    ~CoMesh();

    bool hasAttribute(const VertexAttribute attribute) const;

    bool intersects(const CoRay &ray, IntersectionEvent &intersectionEvent) const;
    Vertex resolveSurface(const IntersectionEvent &intersectionEvent) const;

private:
    class MeshStorage : public CoPrimitiveStorage {
    public:
        MeshStorage(simd::vec3f *positions, vec4u *indices, size_t numFaces);
        ~MeshStorage();

        static constexpr uint32_t kInvalidIndex = -1u;

        size_t NumPrimitives() const;
        CoAxisAlignedBoundingBox PrimitiveBounds(size_t startIdx, size_t endIdx) const;
        size_t
        Reorder(size_t startIdx, size_t endIdx, std::function<bool(const CoAxisAlignedBoundingBox &)> comparator);
        bool PrimitivesIntersect(const CoRay &ray, size_t startIdx, size_t endIdx, IntersectionEvent &event) const;

    private:
        vec4u *_indices;
        size_t _numIndices;

        simd::vec3f *_positions;

        std::vector<CoAxisAlignedBoundingBox> _bounds;

        std::vector<CoAxisAlignedBoundingBox> _ComputePrimitiveBounds(size_t startIdx, size_t endIdx) const;
        friend class CoMesh;
    };

    using MeshAccelerator = CoBoundingVolume<MeshStorage>;

    std::shared_ptr<MeshStorage> _primitives;
    std::unique_ptr<MeshAccelerator> _accelerator;

    CoMesh() = delete;
    CoMesh(const CreateInfo &createInfo);
};

} // namespace cblt::geom

#endif // CBLT_CORE_MESH_H
