#include "triangle_mesh.h"

namespace cblt {
    TriangleMesh::TriangleMesh(std::vector<std::shared_ptr<Triangle>> &tris)
    {
        triangles_ = BoundingVolume<Triangle>(tris);
    }

    bool TriangleMesh::Intersect(const Ray &ray, HitInfo &collision_pt) 
    {
        return triangles_.Intersect(ray, collision_pt); 
    }

    BoundingBox TriangleMesh::GetBounds()
    {
        return triangles_.GetBounds();
    }
}