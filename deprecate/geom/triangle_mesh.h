#ifndef TRIANGLE_MESH_H
#define TRIANGLE_MESH_H

#include "geometry.h"
#include "triangle.h"
#include "bounding_volume.h"

#include <vector>

namespace cblt
{
    class TriangleMesh final : public Geometry
    {
        public:
            TriangleMesh(std::vector<std::shared_ptr<Triangle>> &tris);
            bool Intersect(const Ray &ray, HitInfo &collision_pt) override;
            BoundingBox GetBounds() override;
        private:
            BoundingVolume<Triangle> triangles_;
    };
}

#endif  // TRIANGLE_MESH_H