#include "cook_torrence.h"

#include "math/mat4.h"
#include "math/math_helpers.h"
#include "math/constants.h"

#include "shading_helpers.h"
#include "sampling_helpers.h"
#include "fresnel.h"

#include <cmath>
#include <algorithm>

namespace cblt
{
    CookTorrenceMaterial::CookTorrenceMaterial(Color albedo, Color specular, Color emissive, float ior, float rough, float metal) : 
    albedo_(albedo), specular_(specular), emissive_(emissive), ior_(ior), roughness_(rough), metalness_(metal)
    {
    }

    Color CookTorrenceMaterial::Sample(const Vec3 &outgoing, Vec3 &incoming, float &pdf, const HitInfo &collisionPt, std::shared_ptr<Sampler> &BRDF_sampler)
    {
        // Illuminate using Cook-Torence reflectance model
        // First, determine which BRDF we need to use for this ray
        float x;
        BRDF_sampler->Next1D(x);
        float tot_luminance = 1.f;
        float percent_diffuse = 1.f - metalness_;
        float percent_spec = metalness_;

        // Construct Orthonormal Basis for sampling
        Vec3 bitangent, tangent;
        OrthonormalBasis(collisionPt.norm, bitangent, tangent);

        float u1, u2;
        BRDF_sampler->Next2D(u1, u2);

        if(x <= percent_diffuse)
        {
            // lambertian diffuse BRDF with cosine weighted sampling
            incoming = RandomUnitVectorInCosineWeightedHemisphere(bitangent, collisionPt.norm, tangent, pdf, u1, u2);
        }
        else
        {
            // specular
            incoming = RandomUnitVectorInGGX(bitangent, collisionPt.norm, tangent, outgoing, sqr(roughness_), pdf, u1, u2);
        }
        return BRDF(incoming, outgoing, collisionPt, pdf);
    }

    // nomenclature guide:
    // incoming -> omega_i -> toLight (L) -> The light arriving at this point
    // outgoing -> omega_o -> toEye (V) -> The light reflected at this point
    Color CookTorrenceMaterial::BRDF(const Vec3 &incoming, const Vec3 &outgoing, const HitInfo &collision_pt, float &pdf)
    {
        
        float d_pdf(0.f), s_pdf(0.f);

        float in_dot_n = Dot(incoming, collision_pt.norm);
        float out_dot_n = Dot(outgoing, collision_pt.norm);

        if (in_dot_n < 0.f || out_dot_n < 0.f)
        {
            pdf = 0.f;
            return Color::GreyScale(0.f);
        }

        // lambertian diffuse BRDF
        Color diffuse = (albedo_map_) ? albedo_map_->sample(collision_pt.uv.x, collision_pt.uv.y) : albedo_;
        diffuse = diffuse * INV_PI_f;
        d_pdf = in_dot_n * INV_PI_f;

        // specular
        float rough_sqr = sqr(roughness_);

        Vec3 halfway = Normalize(incoming + outgoing);

        // Approximate microfacets using the GGX normal distribution function
        float n_dot_h = Dot(collision_pt.norm, halfway);
        float o_dot_h = Dot(outgoing, halfway);
        float chi_h_n = static_cast<float>(n_dot_h > 0.f);
        float D = GGX(n_dot_h, rough_sqr);
        // Geometric attenuation - Smith Partial Geometry
        float in_dot_h = Dot(incoming, halfway);
        float G = SmithPartialGeom(incoming, collision_pt.norm, rough_sqr) * SmithPartialGeom(outgoing, collision_pt.norm, rough_sqr);
        // Approximate geometry of microfacets using Schlick
        float F0 = (1.f - ior_) / (1.f + ior_);
        F0 *= F0;
        float F = FresnelSchlick(F0, o_dot_h);
        Color reflective = specular_ * (chi_h_n * D *F * G) / (4.f * std::abs(in_dot_n) * std::abs(out_dot_n));
        
        s_pdf = D * n_dot_h / (4.f * o_dot_h);

        pdf = d_pdf * (1.f - metalness_) + s_pdf * metalness_;
        
        return diffuse * (1.f - metalness_) + reflective * metalness_;
    }

    Color CookTorrenceMaterial::Emittance()
    {
        return emissive_;
    }
}