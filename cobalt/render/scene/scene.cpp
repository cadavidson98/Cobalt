#include "scene.h"

#include "color.h"
#include "texture.h"

#include "geometry/interpolation.h"
#include "geometry/intersection.h"
#include "geometry/mesh.h"
#include "math/constants.h"

#include <atomic>
#include <cmath>
#include <cstring>
#include <memory>

namespace cblt::render {

std::atomic<CoUUID> CoScene::_nextID = std::atomic<CoUUID>(0);

std::shared_ptr<CoScene> CoScene::create(const CoScene::CreateFromDataInfo &createInfo) {
    std::shared_ptr<CoScene> scene = std::shared_ptr<CoScene>(new CoScene);
    scene->_camera = createInfo.camera;
    scene->_environmentMap = createInfo.environmentMap;
    scene->_scenePrimitives.reserve(createInfo.primitives.size());
    scene->_meshes.reserve(createInfo.meshes.size());
    scene->_materials.reserve(createInfo.materials.size());

    std::copy(createInfo.primitives.begin(), createInfo.primitives.end(), std::back_inserter(scene->_scenePrimitives));
    std::copy(createInfo.meshes.begin(), createInfo.meshes.end(), std::back_inserter(scene->_meshes));
    std::copy(createInfo.materials.begin(), createInfo.materials.end(), std::back_inserter(scene->_materials));

    return scene;
}

CoScene::CoScene() {
    const CoMaterial::Properties defaultMaterialProperties{
        .baseColor = CoSpectrum(),
        .metallic = 0.f,
        .subsurface = 0.f,
        .ior = 1.4,
        .specular = 0.f,
        .specularTint = 0.f,
        .specularTransmission = 0.f,
        .roughness = 1.f,
        .anisotropic = 0.f,
        .sheen = 0.f,
        .sheenTint = 0.f,
        .clearcoat = 0.f,
        .clearcoatGloss = 0.f,
    };

    _defaultMaterial = std::make_shared<CoMaterial>(defaultMaterialProperties);
}

CoScene::~CoScene() {
    if (_ptexTextures) {
        _ptexTextures->release();
    }
}

CoUUID CoScene::nextUUID() {
    return std::atomic_fetch_add(&_nextID, 1);
}

std::shared_ptr<CoCamera> CoScene::camera() const {
    return _camera;
}

bool CoScene::closestIntersection(const geom::CoRay &ray, geom::IntersectionEvent &intersectionEvent) const {
    for (size_t idx = 0; idx < _meshes.size(); ++idx) {
        if (_meshes[idx].mesh->intersects(ray, intersectionEvent)) {
            intersectionEvent.geometryIndex = 0;
            return true;
        }
    }
    return false;
}

CoSurfaceParams CoScene::resolveSurfaceAtInteraction(const geom::IntersectionEvent &intersectionEvent) const {
    assert(_scenePrimitives.size());

    const PrimitiveComponents &primitive = _scenePrimitives[intersectionEvent.geometryIndex];
    if (primitive.materialIdx != kInvalidID) {
        // const geom::CoSurface surfaceProperties = _mesh->resolveSurface(intersectionEvent);
        const CoMaterial &material = _materials[primitive.materialIdx];
        return material.surfaceParamsAtCoordinates(
            intersectionEvent.localCoordinates,
            intersectionEvent.primitiveIndex
        );
    }

    return _defaultMaterial->surfaceParamsAtCoordinates(
        intersectionEvent.localCoordinates,
        intersectionEvent.primitiveIndex
    );
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
