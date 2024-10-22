#ifndef CBLT_TRIANGLE_MESH
#define CBLT_TRIANGLE_MESH

#include "geometry/acceleration/bounding_volume.h"
#include "geometry/bounding_box.h"
#include "geometry/intersection/triangle.h"

#include <vector>

namespace cblt {
template<typename vertex>
class TriangleMesh {
    public:
        TriangleMesh(std::vector<vertex> &&vertices, std::vector<uint32_t> &&indices);
        SurfaceLocation Intersect(const Ray &ray, float &time) const;
        AxisAlignedBoundingBox3f Bounds() const;
        SurfaceProperties GetSurfaceProperties(const SurfaceLocation &hitLocation) const;

    private:
        std::vector<vertex> _vertices;
        // each triplet defines a single triangle, zero-indexed, must be a multiple of 3
        std::vector<uint32_t> _indices;
        // each material used in this mesh, each vertex should hold and OFFSET into this array
        std::vector<Material> _materials;
        // what about materials?
        // A: materials should be supplied PER-VERTEX, so
        // they can be layered and LERPed
        // what about vertex weights?
        // A: weights are just another vertex attribute, disregard
        // what about extracting per-vertex data at a hit?
        // A: calculate the hit location, return the triangle INDEX of the hit
        // when we need to extract the final surface properties, we will call back
        // into this mesh to extract the information using the index returned
        // NEW QUESTION: how do we store the return value for the triangle in a meaningful
        // and extendable way? In other words, no casting is allowed and no exposing to other
        // primitive types
        // A: we could store the result in a std::variant then check that the triangle hit info
        // is stored when called from triangle, sphere from sphere, etc...
        // another option would be to create a generic "hitInfo" structure
        // for a sceneEntity, in where there is a field specifically for "child data"
        // for aggregrate entities, like collections or meshes
        AxisAlignedBoundingBox3f _bounds;
        BoundingVolume<triangleIntersect> _acceleration;
};
} // namespace cblt

#endif // CBLT_TRIANGLE_MESH
