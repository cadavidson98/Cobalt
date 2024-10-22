#include "lambertian.h"

#include "shading_helpers.h"
#include "sampling_helpers.h"

namespace cblt
{
    LambertianMaterial::LambertianMaterial(Color base) : base_(base)
    {

    }

    Color LambertianMaterial::Sample(const Vec3 &incoming, Vec3 &outgoing, float &pdf, const HitInfo &collisionPt, std::shared_ptr<Sampler> &BRDF_sampler)
    {
        float u1, u2;
        Vec3 tangent, bitangent;
        
        BRDF_sampler->Next2D(u1, u2);
        OrthonormalBasis(collisionPt.norm, bitangent, tangent);
        
        outgoing = RandomUnitVectorInHemisphere(bitangent, collisionPt.norm, tangent, pdf, u1, u2);
        return BRDF(incoming, outgoing, collisionPt, pdf);
    }

    Color LambertianMaterial::BRDF(const Vec3 &incoming, const Vec3 &outgoing, const HitInfo &collision_pt, float &pdf)
    {
        pdf = .5f * INV_PI_f;
        return base_ * Dot(collision_pt.norm, incoming) * INV_PI_f;
    }

    Color LambertianMaterial::Emittance()
    {
        return Color::GreyScale(0.f);
    }
}