#ifndef CBLT_SCENE_PRIM_H
#define CBLT_SCENE_PRIM_H

#include "boundable.h"
#include "geometry.h"
#include "math\mat4.h"
#include "ray.h"

#include <memory>

namespace cblt
{
    class ScenePrim : public Boundable
    {
        public:
        ScenePrim(const std::shared_ptr<Geometry> &model, const Mat4 &transform /*Material*/);
        BoundingBox GetBounds();
        Ray TransformRay(const Ray &world_ray);
        bool Intersect(const Ray &ray, HitInfo &collision_pt);
        private:
        Mat4 local_to_world_;
        Mat4 world_to_local_;
        std::shared_ptr<Geometry> model_;

        friend class Scene;
    };
}
#endif  // CBLT_SCENE_PRIM_H