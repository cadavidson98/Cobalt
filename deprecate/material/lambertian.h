#ifndef LAMBERTIAN_H
#define LAMBERTIAN_H

#include "material.h"

namespace cblt
{
    class LambertianMaterial : public Material
    {
        public:
        LambertianMaterial(Color base);
        virtual Color Sample(const Vec3 &outgoing, Vec3 &incoming, float &pdf, const HitInfo &collisionPt, std::shared_ptr<Sampler> &BRDF_sampler) override;
        virtual Color BRDF(const Vec3 &outgoing, const Vec3 &incoming, const HitInfo &collision_pt, float &pdf) override;
        virtual Color Emittance() override;
        private:
        Color base_;
    };
}

#endif  // LAMBERTIAN_H