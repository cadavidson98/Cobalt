#include "scene.h"

#include "mat/material.h"
#include "mat/multiple_importance_heuristics.h"

namespace cblt
{
    Scene::Scene(const Camera &camera, std::vector<std::shared_ptr<ScenePrim>> &s_prims, std::vector<std::shared_ptr<Light>> &l_prims)
    {
        cam_ = camera;
        
        s_prims_ = s_prims;
        l_prims_ = l_prims;
        accel_ = BoundingVolume<ScenePrim>(s_prims_);
        // TODO - materials: Should the pointers also be held here, or just in the primitives?
    }

    void Scene::AddPrim(const std::shared_ptr<ScenePrim> &s_prim)
    {
        s_prims_.push_back(s_prim);
        // TODO - rebuild BVH?
    }

    void Scene::AddPrim(const std::shared_ptr<Light> &l_prim)
    {
        l_prims_.push_back(l_prim);
    }

    bool Scene::Intersects(const Ray &ray, float &time)
    {
        HitInfo collision_pt;
        collision_pt.hit_time = inf_F;
        bool hit = accel_.Intersect(ray, collision_pt);
        time = collision_pt.hit_time;
        return hit;
    }

    bool Scene::ClosestIntersection(const Ray &ray, HitInfo &collision_pt)
    {
        return accel_.Intersect(ray, collision_pt);
    }

    Color Scene::SampleSingleLight(const Vec3 &outgoing, const HitInfo &collision_pt, std::shared_ptr<Sampler> &sampler)
    {
        if (l_prims_.size() == 0)
        {
            return Color::GreyScale(0.f);
        }
        
        // choose a random light from all light sources in the scene
        float light_num, light_pdf, light_len;
        sampler->Next1D(light_num);

        int light_idx = static_cast<int>(std::round(light_num * l_prims_.size()));
        light_idx = std::min(static_cast<int>(l_prims_.size()) - 1, light_idx);
        std::shared_ptr<Light> &selected_light = l_prims_[light_idx];

        Color radiance = Color::GreyScale(0.f);

        radiance = radiance + DirectLight(outgoing, selected_light, collision_pt, sampler) * l_prims_.size();

        return radiance;
    }

    Color Scene::DirectLight(const Vec3 &outgoing, std::shared_ptr<Light> &light, const HitInfo &collision_pt, std::shared_ptr<Sampler> &sampler)
    {
        Color radiance = Color::GreyScale(0.f);
        Vec3 to_light;
        float light_len, light_pdf;
        Color light_rad = light->Sample(to_light, collision_pt.pos, collision_pt.norm, light_len, light_pdf, sampler);
        
        light_rad = light_rad * AbsDot(collision_pt.norm, to_light);
        
        // check to see if the light is occuluded or not
        HitInfo occluder_info;
        occluder_info.hit_time = inf_F;
        
        Ray shadow_ray(collision_pt.pos + to_light * eps_zero_F, to_light);
        bool hit = ClosestIntersection(shadow_ray, occluder_info);
        if (light_rad.Luminance() <= eps_zero_F || (light_len - occluder_info.hit_time) > eps_zero_F)
        {
            radiance = Color::GreyScale(0.f);
        }
        else
        {
            float brdf_pdf;
            Color surf_refl = collision_pt.m->BRDF(to_light, outgoing, collision_pt, brdf_pdf);

            if (light->isDiracDelta())
            {
                // can't multiple importance sample, since there is a dirac delta, so just return now
                return surf_refl * light_rad / light_pdf;
            }
            else
            {
                // Multiple importance sample
                float MIS = PowerHeuristic(light_pdf, brdf_pdf, 2.f);
                radiance = surf_refl * light_rad * MIS / light_pdf;
            }
        }
        if (light->isDiracDelta())
        {
            // prevent multiple importance sampling when the light is a dirac delta
            // BUT is occluded
            return radiance;
        }
        
        // now sample the BRDF
        Vec3 brdf_dir;
        float brdf_pdf;
        Color surf_refl = collision_pt.m->Sample(outgoing, brdf_dir, brdf_pdf, collision_pt, sampler);
        surf_refl = surf_refl * AbsDot(brdf_dir, collision_pt.norm);
        
        // check if we hit the light source after leaving this sample
        std::shared_ptr<Geometry> area_light_geom = std::dynamic_pointer_cast<cblt::Geometry>(light);
        if (area_light_geom.get())
        {
            // this is an area light - need to do extra geometry checks
            float light_pdf = 0.f;
            
            Ray brdf_ray(collision_pt.pos + brdf_dir * eps_zero_F, brdf_dir);
            HitInfo light_info;
            occluder_info.hit_time = inf_F;

            bool hit_scene = ClosestIntersection(brdf_ray, occluder_info);
            bool hit_light = area_light_geom->Intersect(brdf_ray, light_info);
            light_rad = light->Radiance(light_info.pos, collision_pt.pos, collision_pt.norm, light_pdf);
            if (!hit_light || light_pdf == 0.f || (light_info.hit_time - occluder_info.hit_time) > eps_zero_F)
            {
                // didn't hit the light source
                return radiance;
            }
            
            // compute direct lighting using MIS again
            float MIS = PowerHeuristic(brdf_pdf, light_pdf, 2.f);
            radiance = radiance + (surf_refl * light_rad * MIS / brdf_pdf);
        }
        else
        {
            // this is not an area light
            // currently, there are only area lights and direction lights, the latter which
            // is a dirac delta. Therefore, the code should never reach this point, but that
            // may change in the future if I add environment maps & IBL.
        }
        return radiance;
    }
}