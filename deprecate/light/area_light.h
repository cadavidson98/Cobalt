#ifndef CBLT_AREA_LIGHT
#define CBLT_AREA_LIGHT

#include "light.h"
#include "geom/geometry.h"

namespace cblt
{
    class AreaLight : public Light, public Geometry
    {
        public:
        AreaLight(const Vec3 &pos, const Vec3 &X, const Vec3 &Y, const Vec3 &Z, const Color &clr, float lumens, float length, float width);

        // since area lights have ... area ..., they are possible to
        // intersect during path tracing, so we must inherit from geometry
        bool Intersect(const Ray &ray, HitInfo &collision_pt) override;
        BoundingBox GetBounds() override;

        Color Sample(Vec3 &to_light, const Vec3 &surf_pos, const Vec3 &surf_norm, float &dist, float &pdf, std::shared_ptr<Sampler> &sampler) override;
        Color Radiance(const Vec3 &light_pos, const Vec3 &surf_pos, const Vec3 &surf_norm, float &pdf) override;
        private:
        
        // area light radiance info
        Color color_;
        float length_;
        float width_;
        float power_;
        float area_;

        // area light basis info
        Vec3 pos_;
        Vec3 dir_X_;
        Vec3 dir_Y_;
        Vec3 dir_Z_;
        
    };
}

#endif  // CBLT_AREA_LIGHT