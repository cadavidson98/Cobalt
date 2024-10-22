#include "direction_light.h"

#include "math/constants.h"

#include "mat/shading_helpers.h"
#include "mat/sampling_helpers.h"

namespace cblt
{
    DirectionLight::DirectionLight(Vec3 &dir, Color &clr, float pwr, float angular_spread) : color_(clr), power_(pwr), spread_(angular_spread)
    {
        spread_ = spread_;
        dir_ = Normalize(dir);
        radius_ = std::tan(spread_ * .5f);
    }
    
    Color DirectionLight::Sample(Vec3 &to_light, const Vec3 &surf_pos, const Vec3 &surf_norm, float &dist, float &pdf, std::shared_ptr<Sampler> &sampler)
    {
        if (spread_ > eps_zero_F)
        {
            Vec3 tangent, bitangent;
            OrthonormalBasis(dir_, tangent, bitangent);
            float x, y;
            sampler->Next2D(x, y);
            
            Vec2 disk_sample = ConcentricPointOnUnitDisk(x, y) * radius_;
            to_light = Normalize(-dir_ + tangent * disk_sample.x + bitangent * disk_sample.y);
            // since our spread is VERY narrow, it is reasonable to assume our PDF doesn't change
            pdf = 1.f;
            // alternate idea - directional spread is akin to sampling a very narrow cone
            // pdf = 1.f / (2.f * PI_f * (1.f - spread_ * .5f));
            dist = inf_F;
        }
        else
        {
            to_light = -dir_;
            dist = inf_F;
            pdf = 1.f;
        }
        return color_ * power_;
    }

    Color DirectionLight::Radiance(const Vec3 &light_pos, const Vec3 &surf_pos, const Vec3 &surf_norm, float &pdf)
    {
        if (spread_ > eps_zero_F)
        {
            pdf = 1.f / (2.f * PI_f * (1.f - spread_ * .5f));
        }
        else
        {
            pdf = 1.f;
        }
        
        return color_ * power_ / PI_f;
    }

    bool DirectionLight::isDiracDelta()
    {
        return true;
    }
}