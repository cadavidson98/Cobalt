#ifndef CBLT_LIGHT_PRIM_H
#define CBLT_LIGHT_PRIM_H

#include "light/light.h"
#include "math/mat4.h"

#include <memory>

namespace cblt
{
    class LightPrim
    {
        public:
        LightPrim(std::shared_ptr<Light> &light, Mat4 instance_transform);
        // obtain a random "to light" vector which points to a random point on the light surface
        Color SampleLight(Vec3 &incoming, const Vec3 &outgoing, float &length, float &pdf, const HitInfo &collision_pt, std::shared_ptr<Sampler> &sampler);
        Color Radiance(const Vec3 &light_pos, const Vec3 &surf_pos, const Vec3 &surf_norm);
        Color Emission();
        private:
        std::shared_ptr<Light> light_;
        Mat4 local_to_world_;
        Mat4 world_to_local_;

        friend class Scene;
    };
}

#endif  // CBLT_LIGHT_PRIM_H