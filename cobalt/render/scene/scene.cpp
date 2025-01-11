#include "scene.h"

#include "color.h"
#include "constants.h"
#include "intersection.h"
#include "mesh.h"
#include "texture.h"

#include <cmath>
#include <memory>

namespace cblt::render {

std::shared_ptr<CoScene> CoScene::Create(const CoScene::CreateFromDataInfo &createInfo) {
    std::shared_ptr<CoScene> scene = std::shared_ptr<CoScene>(new CoScene);
    scene->_camera = createInfo.camera;
    scene->_environmentMap = createInfo.environmentMap;
    scene->_mesh = createInfo.mesh;
    return scene;
}

CoScene::CoScene(): _camera{{}} {
}

CoScene::~CoScene() {
}

bool CoScene::closestIntersection(const geom::CoRay &ray, geom::IntersectionEvent &intersectionEvent) const {
    return _mesh->intersects(ray, intersectionEvent);
}

CoMaterial *CoScene::materialForPrimitive(CoUUID primitiveID) const {
    const PrimitiveComponents &primitive = _scenePrimitives[primitiveID];
    return (primitive.materialIdx != kInvalidID) ? _materials[primitive.materialIdx] : nullptr;
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
