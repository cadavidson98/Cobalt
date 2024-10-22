#include "light_prim.h"
#include "math/constants.h"

#include "mat/image_lib.h"
#include "mat/material.h"
#include "mat/multiple_importance_heuristics.h"


namespace cblt
{
    LightPrim::LightPrim(std::shared_ptr<Light> &light, Mat4 instance_transform) :
    light_(light)
    {
        local_to_world_ = instance_transform;
        world_to_local_ = Inverse(local_to_world_); 
    }

    Color LightPrim::SampleLight(Vec3 &incoming, const Vec3 &outgoing, float &length, float &pdf, const HitInfo &collision_pt, std::shared_ptr<Sampler> &sampler)
    {
        /*Vec4 light_sample;
        Color light_clr = light_->Sample(light_sample, pdf, sampler);
        // transform to the world space position and create the "to light" vector
        Vec4 world_sample = local_to_world_ * light_sample;
        float inv_dist_sqr = 1.f;
        // directional
        if (light_sample.w == 0.f)
        {
            incoming = Normalize(Vec3(world_sample.x, world_sample.y, world_sample.z));
            length = inf_F;
        }
        // positional
        else
        {
            Vec4 world_dir = local_to_world_ * Vec4(Y_axis_F, 0.f);
            incoming = Vec3(world_sample.x, world_sample.y, world_sample.z) - collision_pt.pos;
            length = Magnitude(incoming);
            incoming = incoming / length;
            inv_dist_sqr = std::max(0.f, -Dot(incoming, Normalize(Vec3(world_dir.x, world_dir.y, world_dir.z)))) / (length * length);
        }
        // compute radiance here, will mix in the BRDF & Visibility in the calling code
        return light_clr * inv_dist_sqr;*/
        return Color::GreyScale(0.f);
    }

    Color LightPrim::Radiance(const Vec3 &light_pos, const Vec3 &surf_pos, const Vec3 &surf_norm)
    {
        /*
        Vec4 local_light_pos = world_to_local_ * Vec4(light_pos, 1.f);
        Vec4 local_surf_pos = world_to_local_ * Vec4(surf_pos, 1.f);
        Vec4 local_surf_norm = world_to_local_ * Vec4(surf_norm, 0.f);

        Vec3 l_pos = { local_light_pos.x, local_light_pos.y, local_light_pos.z };
        Vec3 s_pos = { local_surf_pos.x, local_surf_pos.y, local_surf_pos.z };
        Vec3 s_norm = { local_surf_norm.x, local_surf_norm.y, local_surf_norm.z };

        return light_->Radiance(l_pos, s_pos, s_norm);
        */
        return Color::GreyScale(0.f);
    }

    Color LightPrim::Emission()
    {
        return Color::GreyScale(0.f);
    }
}