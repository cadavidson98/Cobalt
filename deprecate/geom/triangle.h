#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "geometry.h"
#include "math/vec.h"
#include "mat/material.h"

#include <memory>

namespace cblt
{
    class Triangle final : public Geometry
    {
        public:
            Triangle();
            Triangle(const Vec3 &p1, const Vec3 &p2, const Vec3 &p3, std::shared_ptr<Material> &mat);
            Triangle(const Vec3 &p1, const Vec3 &p2, const Vec3 &p3, 
                     const Vec3 &n1, const Vec3 &n2, const Vec3 &n3, std::shared_ptr<Material> &mat);
            Triangle(const Vec3 &p1, const Vec3 &p2, const Vec3 &p3, 
                     const Vec3 &n1, const Vec3 &n2, const Vec3 &n3,
                     const Vec2 &uv1, const Vec2 &uv2, const Vec2 &uv3, std::shared_ptr<Material> &mat);
            bool Intersect(const Ray &ray, HitInfo &collision_pt) override;
            BoundingBox GetBounds() override;
        private:

            // position attributes
            Vec3 pos1_;
            Vec3 pos2_;
            Vec3 pos3_;

            // normal attributes
            Vec3 face_norm_;
            float face_norm_len_;
            bool use_vertex_norms_;
            Vec3 norm1_;
            Vec3 norm2_;
            Vec3 norm3_;

            // uv attributes
            bool use_vertex_uvs_;
            Vec2 uv1_;
            Vec2 uv2_;
            Vec2 uv3_;

            // material
            std::shared_ptr<Material> mat_;
    };
}

#endif  // TRIANGLE_H