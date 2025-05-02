#include "scene.h"

#include "color.h"
#include "resolver.h"
#include "texture.h"
#include "texture_cache.h"

#include "core/logging.h"
#include "geometry/intersection.h"
#include "geometry/mesh.h"
#include "math/constants.h"

#include <atomic>
#include <cmath>
#include <cstring>
#include <memory>

namespace cblt::render {

std::atomic<CoUUID> CoScene::_nextID = std::atomic<CoUUID>(0);

std::shared_ptr<CoScene> CoScene::create(const CoScene::CreateInfo &createInfo) {
    std::shared_ptr<CoScene> scene = std::shared_ptr<CoScene>(new CoScene(createInfo));

    return scene;
}

CoScene::CoScene(const CreateInfo &createInfo) {
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

    // TODO: promote selecting default material to initialization; then we don't need an 'if' statement here
    _defaultMaterial = std::make_shared<CoMaterial>(defaultMaterialProperties);

    _camera = createInfo.camera;
    _environmentMap = createInfo.environmentMap;
    _scenePrimitives.reserve(createInfo.primitives.size());
    _geometries.reserve(createInfo.meshes.size());
    _materials.reserve(createInfo.materials.size());

    std::copy(createInfo.primitives.begin(), createInfo.primitives.end(), std::back_inserter(_scenePrimitives));
    std::copy(createInfo.meshes.begin(), createInfo.meshes.end(), std::back_inserter(_geometries));
    std::copy(createInfo.materials.begin(), createInfo.materials.end(), std::back_inserter(_materials));

    static constexpr size_t kGBToMB = 1024;
    _textureCache = CoTextureCache::create({
        .baseDirectory = createInfo.options.baseDirectory,
        .cacheSizeMB = 2 * kGBToMB,
    });
}

CoScene::~CoScene() {
}

CoUUID CoScene::nextUUID() {
    return std::atomic_fetch_add(&_nextID, 1);
}

std::shared_ptr<CoCamera> CoScene::camera() const {
    return _camera;
}

bool CoScene::closestIntersection(const geom::CoRay &ray, geom::IntersectionEvent &intersectionEvent) const {
    for (size_t idx = 0; idx < _scenePrimitives.size(); ++idx) {
        if (_scenePrimitives[idx].geometryIdx == kInvalidID) {
            continue;
        }

        const CoUUID &geometryIdx = _scenePrimitives[idx].geometryIdx;

        if (_geometries[geometryIdx].mesh->intersects(ray, intersectionEvent)) {
            intersectionEvent.geometryIndex = idx;
            return true;
        }
    }
    return false;
}

std::optional<CoSurfaceParams> CoScene::resolveSurfaceAtInteraction(const geom::IntersectionEvent &intersectionEvent
) const {
    assert(_scenePrimitives.size());

    const Primitive &primitive = _scenePrimitives[intersectionEvent.geometryIndex];

    if (primitive.geometryIdx == kInvalidID || primitive.materialIdx == kInvalidID) {
        CoLogError("Primitive cannot be resolved: missing material and/or geometry components");
        return std::nullopt;
    }

    const GeometryComponent &geometry = _geometries[primitive.geometryIdx];
    const MaterialComponent &material = _materials[primitive.materialIdx];

    const std::optional<CoResolvedSurface> surface =
        material.resolver->resolve(*geometry.mesh, *material.surface, intersectionEvent);

    return surface ? std::make_optional<CoSurfaceParams>(surface->surfaceParams) : std::nullopt;
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
