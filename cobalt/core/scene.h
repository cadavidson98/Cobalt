#ifndef CBLT_SCENE_H
#define CBLT_SCENE_H

#include "bounding_volume.h"
#include "camera.h"
#include "geometry/geometry.h"
#include "light/light.h"
#include "material/material.h"
#include "scene_prim.h"

#include <memory>
#include <vector>

namespace cblt {
struct SceneEntity {
        uint32_t geometryComponent;
        uint32_t lightComponent;
        Transform localToWorld;
        Transform worldToLocal;
};

class Scene {
    public:
        Scene(
            const Camera &camera,
            std::vector<std::shared_ptr<ScenePrim>> &s_prims,
            std::vector<std::shared_ptr<Light>> &l_prims
        );
        bool Intersects(const Ray &ray, float &time);
        bool ClosestIntersection(const Ray &ray, HitInfo &collision_pt);
        Color SampleSingleLight(const Vec3 &outgoing, const HitInfo &collision_pt, std::shared_ptr<Sampler> &sampler);
        Color DirectLight(
            const Vec3 &outgoing,
            std::shared_ptr<Light> &light,
            const HitInfo &collision_pt,
            std::shared_ptr<Sampler> &sampler
        );
        Camera cam_;

    private:
        // scene contains...
        // lights (emmissive) (duh)
        std::vector<LightSource> _lights;
        // geometry (intersectable) (duh)
        std::vector<Geometry> _geometry;
        // materials (not as obvious)
        std::vector<Material> _materials;
        // acceleration for lookup
        BoundingVolume<SceneEntity> _accelerationBVH;
        // no need for camera
        // some items are lights & geometry
        std::vector<std::shared_ptr<Light>> l_prims_;
        std::vector<std::shared_ptr<ScenePrim>> s_prims_;
        BoundingVolume<ScenePrim> accel_;
};
} // namespace cblt

#endif // CBLT_SCENE_H
