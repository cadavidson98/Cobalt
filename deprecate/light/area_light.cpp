#include "area_light.h"

#include <algorithm>

namespace cblt
{
    AreaLight::AreaLight(const Vec3 &pos, const Vec3 &X, const Vec3 &Y, const Vec3 &Z, 
                         const Color &clr, float energy, float length, float width) :
                         pos_(pos), dir_X_(X), dir_Y_(Y), dir_Z_(Z), 
                         color_(clr), length_(length), width_(width)
    {
        area_ = length_ * width_;
        power_ = energy / (PI_f * area_);
    }

    Color AreaLight::Sample(Vec3 &to_light, const Vec3 &surf_pos, const Vec3 &surf_norm, float &dist, float &pdf, std::shared_ptr<Sampler> &sampler)
    {
        float u, v;
        // samples return values in the range of [0, 1], so to center the area light 
        // at the origin, we need to shift the samples to [-.5, .5]
        sampler->Next2D(u, v);
        float x = length_ * (u - .5f);
        // y axis is up, which is the light direction
        float z = width_ * (v - .5f);
        Vec3 light_pos = pos_ + dir_X_ * x + dir_Z_ * z;
        // to light is un-normalized, 
        to_light = light_pos - surf_pos;
        float len_sqr = MagnitudeSqr(to_light);
        dist = std::sqrt(len_sqr);
        to_light = to_light / dist;
        pdf = 1.f / area_;
        return color_ * power_ * std::max(0.f, -Dot(to_light, dir_Y_)) / (len_sqr);
    }

    bool AreaLight::Intersect(const Ray &ray, HitInfo &collision_pt)
    {
        float dt = Dot(ray.dir, dir_Y_);
        if (std::abs(dt) < eps_zero_F)
        {
  	        // the ray is parallel, so return false
  	        return false;
  	    }

  	    float hit_t = Dot(pos_ - ray.pos, dir_Y_) / dt;
  	    
        if (hit_t < eps_zero_F)
        {
  	        // only go forward in the direction
  	        return false;
  	    }
	
  	    Vec3 hit_pos = ray.pos + ray.dir * hit_t;
        // use same-sided test to check that our point is in the quad
        Vec3 edge1 = dir_X_ * length_;
        Vec3 edge2 = dir_Z_ * width_;
        Vec3 edge3 = -edge1;
        Vec3 edge4 = -edge2;

        Vec3 A = hit_pos - pos_ + (edge3 + edge4) * .5f;
        Vec3 B = hit_pos - pos_ + (edge1 + edge4) * .5f;
        Vec3 C = hit_pos - pos_ + (edge1 + edge2) * .5f;
        Vec3 D = hit_pos - pos_ + (edge3 + edge2) * .5f;
        
        bool in = Dot(A, edge1) >= 0.f &&
                  Dot(B, edge2) >= 0.f &&
                  Dot(C, edge3) >= 0.f &&
                  Dot(D, edge4) >= 0.f;

        if (in)
        {
            collision_pt.hit_time = hit_t;
            collision_pt.norm = dir_Y_;
            collision_pt.pos = hit_pos;
        }
        return in;
    }

    BoundingBox AreaLight::GetBounds()
    {
        Vec3 min_pt(-.5f * length_, 0.f, -.5f * width_), 
             max_pt( .5f * length_, 0.f,  .5f * width_);
        return BoundingBox(min_pt, max_pt);
    }

    Color AreaLight::Radiance(const Vec3 &light_pos, const Vec3 &surf_pos, const Vec3 &surf_norm, float &pdf)
    {
        // Area lighting Integral from Sebastian Legarde "Moving Frostbite to PBR 3.0"
        // We need to account for the distance attenuation when directly sampling the area light
        Vec3 to_light = light_pos - surf_pos;
        float length_sqr = MagnitudeSqr(to_light);
        // Normalize
        to_light = to_light / std::sqrt(length_sqr);
        pdf = 1.f / area_;
        return color_ * power_ * std::max(0.f, -Dot(to_light, dir_Y_)) / length_sqr;
    }
}