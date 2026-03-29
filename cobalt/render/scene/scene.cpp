#include "scene.h"

namespace cobalt::render {

std::shared_ptr<Scene> Scene::create(const Scene::CreateInfo &createInfo) {
    std::shared_ptr<Scene> scene = std::shared_ptr<Scene>(new Scene(createInfo));

    return scene;
}

Scene::Scene(const CreateInfo &createInfo)
    : _camera{createInfo.camera}, _environmentMap{createInfo.environmentMap}, _geometry{createInfo.geometry},
      _components{createInfo.components} {
}

std::shared_ptr<Camera> Scene::camera() const {
    return _camera;
}

std::shared_ptr<Texture> Scene::environmentMap() const {
    return _environmentMap;
}

std::shared_ptr<geom::SceneStorage> Scene::storage() const {
    return _geometry;
};

std::shared_ptr<ComponentStorage> Scene::componentStorage() const {
    return _components;
}

} // namespace cobalt::render
