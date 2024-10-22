#ifndef CBLT_DIRECTION_LIGHT_H
#define CBLT_DIRECTION_LIGHT_H

#include "light.h"

namespace cblt
{
    class DirectionLight : public Light
    {
        public:
        DirectionLight(Vec3 &dir, Color &clr, float pwr, float anglular_spread = 0.f);
        Color Sample(Vec3 &to_light, const Vec3 &surf_pos, const Vec3 &surf_norm, float &dist, float &pdf, std::shared_ptr<Sampler> &sampler) override;
        Color Radiance(const Vec3 &light_pos, const Vec3 &surf_pos, const Vec3 &surf_norm, float &pdf) override;
        bool isDiracDelta() override;
        private:
        Color color_;
        Vec3 dir_;
        float power_;
        float spread_;
        float radius_;
    };
}

#endif  // CBLT_DIRECTION_LIGHT