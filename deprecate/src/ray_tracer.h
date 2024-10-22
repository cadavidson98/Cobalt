#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "image_lib.h"

#include "geom/scene.h"
#include "sampler.h"

#include <string>
#include <vector>
#include <memory>

struct RenderSettings
{
    int img_width;
    int img_height;
    int num_samples;
    int num_threads;
    int path_depth;
    int tile_size;
};

class RayTracer
{
public:
    RayTracer(int width, int height, std::shared_ptr<cblt::Scene> &image_scene_);
    RayTracer(const RenderSettings &settings, std::shared_ptr<cblt::Scene> &image_scene_);
    ~RayTracer();
    std::shared_ptr<Image> Render();
private:
    RenderSettings image_settings_;
    std::shared_ptr<cblt::Scene> image_scene_;

    Color PathTraceIterative(cblt::Ray cam_ray, std::shared_ptr<cblt::Sampler> &generator);
    bool SceneIntersect(cblt::Ray ray, cblt::HitInfo &hit);
};

#endif  // RAYTRACER_H