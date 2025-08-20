#include "scene.h"

#include "color.h"
#include "texture.h"
#include "texture_cache.h"

#include "geometry/bounding_volume_crtp.h"
#include "geometry/bounding_volume_scene_storage.h"
#include "geometry/bounding_volume_types.h"
#include "geometry/intersection.h"
#include "geometry/mesh.h"
#include "math/constants.h"

#include <cmath>
#include <cstring>
#include <memory>

namespace cblt::render {

std::shared_ptr<CoScene> CoScene::create(const CoScene::CreateInfo &createInfo) {
    std::shared_ptr<CoScene> scene = std::shared_ptr<CoScene>(new CoScene(createInfo));

    return scene;
}

CoScene::CoScene(const CreateInfo &createInfo) {

    _camera = createInfo.camera;
    _environmentMap = createInfo.environmentMap;
    _scenePrimitives.reserve(createInfo.primitives.size());
    _materials.reserve(createInfo.materials.size());

    _geometry = createInfo.geometry;
    _accelerator = std::make_unique<SceneAccelerator>(SceneAccelerator::CreateWithPrimitivesInfo{
        .primitives = _geometry,
    });

    std::copy(createInfo.primitives.begin(), createInfo.primitives.end(), std::back_inserter(_scenePrimitives));
    std::copy(createInfo.materials.begin(), createInfo.materials.end(), std::back_inserter(_materials));

    static constexpr size_t kGBToMB = 1024;
    _textureCache = CoTextureCache::create({
        .baseDirectory = createInfo.options.baseDirectory,
        .cacheSizeMB = 2 * kGBToMB,
    });
}

CoScene::~CoScene() {
}

std::shared_ptr<CoCamera> CoScene::camera() const {
    return _camera;
}

bool CoScene::closestIntersection(const geom::CoRay &ray, geom::IntersectionEvent &intersectionEvent) const {
    const geom::crtp::IntersectionResult result = _accelerator->intersects(ray);
    intersectionEvent.timeMin = result.hitTime;
    return result.primitive.type != geom::crtp::PrimitiveType::kNone;
}

CoColor CoScene::environment(const geom::CoRay &ray) const {
    const float phi = std::acos(ray.dir[1]);
    // TODO: make sure camera is using an rhs csys
    float theta = std::atan2(-ray.dir[2], ray.dir[0]);
    theta = (theta < 0.f) ? theta + cblt::kPI : theta;
    const float u = ((theta) / (2.f * cblt::kPI));
    const float v = phi / cblt::kPI;

    return _environmentMap->sample({u, v});
}

} // namespace cblt::render
