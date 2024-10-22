#ifndef CBLT_COOK_TORRENCE_H
#define CBLT_COOK_TORRENCE_H

#include "material.h"
#include <memory>

namespace cblt
{
    class CookTorrenceMaterial final : public Material {
    public:
        CookTorrenceMaterial(Color albedo, Color specular, Color emissive, float ior, float rough_, float metal);

        Color Sample(const Vec3 &incoming, Vec3 &outgoing, float &pdf, const HitInfo &collisionPt, std::shared_ptr<Sampler> &BRDF_sampler);
        Color BRDF(const Vec3 &incoming, const Vec3 &outgoing, const HitInfo &collision_pt, float &pdf);
        Color Emittance();
    private:
        Color albedo_;
        Color specular_;
        Color emissive_;
        float ior_;
        float roughness_;
        float metalness_;
    };
}
#endif  // CBLT_COOK_TORRENCE_H