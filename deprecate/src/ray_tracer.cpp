#include "ray_tracer.h"

#include <cmath>
#include <algorithm>
#include <omp.h>

#include "math/math_helpers.h"
#include "math/constants.h"

#include "mat/material.h"
#include "mat/random_sampler.h"
#include "mat/sobol2D.h"

#include <iostream>

RayTracer::RayTracer(int width, int height, std::shared_ptr<cblt::Scene> &scene_data)
{
    image_scene_ = scene_data;

    image_settings_.img_width = width;
    image_settings_.img_height = height;
    
    image_settings_.path_depth = 8;
    image_settings_.num_samples = 32;

    image_settings_.num_threads = 10;
    image_settings_.tile_size = 32;
}

RayTracer::RayTracer(const RenderSettings &settings, std::shared_ptr<cblt::Scene> &scene_data)
{
    image_scene_ = scene_data;
    image_settings_ = settings;
}

RayTracer::~RayTracer()
{
}

std::shared_ptr<Image> RayTracer::Render()
{
    std::shared_ptr<Image> result = std::make_shared<Image>(image_settings_.img_width, image_settings_.img_height);
    float half_width = image_settings_.img_width * .5f;
    float half_height = image_settings_.img_height * .5f;
    
    float frac = 1.f / static_cast<float>(image_settings_.num_samples);
    
    omp_set_num_threads(image_settings_.num_threads);
    // prepare tiles
    struct RenderTileWork
    {
        int start_x;
        int start_y;
        int count_x;
        int count_y;
        RenderTileWork(int s_x, int s_y, int c_x, int c_y)
        {
            start_x = s_x;
            start_y = s_y;
            count_x = c_x;
            count_y = c_y;
        }
    };

    std::vector<RenderTileWork> tiles;

    int t_size = image_settings_.tile_size;
    for (int y = 0; y < image_settings_.img_height; y += t_size)
    {
        for (int x = 0; x < image_settings_.img_width; x += t_size)
        {
            tiles.emplace_back(x, y, std::min(t_size, image_settings_.img_width - x), std::min(t_size, image_settings_.img_height - y));
        }
    }

    int tiles_complete = 0;
    int tiles_tot = tiles.size();

    std::cout << "Tiles: " << tiles_complete << "/" << tiles_tot << std::flush;

    #ifndef _DEBUG
    #pragma omp parallel
    #endif
    {
        std::shared_ptr<cblt::Sampler> generator = std::make_shared<cblt::RandomSampler>(omp_get_thread_num() * 1234U);
        int tile_count = 0;
        #ifndef _DEBUG
        #pragma omp for schedule(guided) nowait
        #endif
        for (int idx = 0; idx < tiles.size(); ++idx)
        {  
            RenderTileWork &cur_tile = tiles[idx];
            for (int pixel_y = 0; pixel_y < cur_tile.count_y; ++pixel_y)
            {
                for (int pixel_x = 0; pixel_x < cur_tile.count_x; ++pixel_x)
                {
                    int x = pixel_x + cur_tile.start_x;
                    int y = pixel_y + cur_tile.start_y;
                    Color tot_clr(0, 0, 0);
                    for (int i = 1; i <= image_settings_.num_samples; i++)
                    {
                        float jitter_x, jitter_y;
                        generator->Next2D(jitter_x, jitter_y);

                        float u = half_width - (x + jitter_x);
                        float v = half_height - (y + jitter_y);

                        cblt::Ray ray = image_scene_->cam_.CreateRay(u, v);
                        tot_clr = tot_clr + PathTraceIterative(ray, generator);
                    }
                    tot_clr = tot_clr * frac;
                    // Reihard Tone Map
                    Color hdr = tot_clr / (Color(1.f, 1.f, 1.f) + tot_clr);
                    result->setPixel(x, y, Color(std::pow(hdr.r, 1.f / 2.2f), std::pow(hdr.g, 1.f / 2.2f), std::pow(hdr.b, 1.f / 2.2f)));
                }
            }
            ++tile_count;
            if (tile_count == 10)
            {
                #ifndef _DEBUG
                #pragma omp critical
                #endif
                {
                    tiles_complete += 10;
                    std::cout << "\rTiles: " << tiles_complete << "/" << tiles_tot << std::flush;
                }
                tile_count = 0;
            }
        }
        #ifndef _DEBUG
        #pragma omp critical
        #endif
        {
            tiles_complete += tile_count;
            std::cout << "\rTiles: " << tiles_complete << "/" << tiles_tot << std::flush;
        }
    }
    return result;
}

Color RayTracer::PathTraceIterative(cblt::Ray cam_ray, std::shared_ptr<cblt::Sampler> &generator)
{
    Color tot_light(0.f, 0.f, 0.f), throughput(1.f, 1.f, 1.f);
    cblt::Ray path_ray = cam_ray;
    for (int depth = 0; depth < image_settings_.path_depth; ++depth)
    {
        // intersect scene
        cblt::HitInfo scene_pt;
        scene_pt.hit_time = cblt::inf_F;
        bool hit = SceneIntersect(path_ray, scene_pt);
        
        if (!hit) 
        {
            break;
        }

        // compute direct lighting contribution
        tot_light = tot_light + throughput * image_scene_->SampleSingleLight(-path_ray.dir, scene_pt, generator);
        
        // sample BRDF at point to determine how light is transmitted to the next point on the path
        cblt::Vec3 incoming;
        float pdf;
        Color f = scene_pt.m->Sample(-path_ray.dir, incoming, pdf, scene_pt, generator);
        if (f.r + f.g + f.b < cblt::eps_zero_F)
        {
            break;
        }
        
        float cos_theta = cblt::AbsDot(scene_pt.norm, incoming);
        throughput = throughput * f * cos_theta / pdf;
        
        path_ray = cblt::Ray(scene_pt.pos + incoming * cblt::eps_zero_F, incoming);

        // russian roulette
        if (depth > 3)
        {
            float p = throughput.Luminance();
            float cutoff;
            generator->Next1D(cutoff);
            if(cutoff > p)
            {
                break;
            }
            throughput = throughput / p;
        }
    }
    return tot_light;
}

// Check if the ray intersects the scene.
bool RayTracer::SceneIntersect(cblt::Ray ray, cblt::HitInfo &hit)
{
    return image_scene_->ClosestIntersection(ray, hit);
}