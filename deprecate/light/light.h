#ifndef CBLT_LIGHT_H
#define CBLT_LIGHT_H

#include "math/vec.h"

#include "mat/sampler.h"
#include "mat/image_lib.h"

#include "geom/hit_info.h"

namespace cblt
{
    class Light
    {
        public:
        // obtain a random point on the light's surface OR a light direction vector (depending on the light type)
        virtual Color Sample(Vec3 &to_light, const Vec3 &surf_pos, const Vec3 &surf_norm, float &dist, float &pdf, std::shared_ptr<Sampler> &sampler) = 0;
        virtual Color Radiance(const Vec3 &light_pos, const Vec3 &surf_pos, const Vec3 &surf_norm, float &pdf) = 0;
        // I want to add support for directional lights in addition to area lights, so
        // we will need to take care when integrating over the light source (and sample)
        virtual bool isDiracDelta()
        {
            return false;
        }
    };
}

#endif  // CBLT_LIGHT_H