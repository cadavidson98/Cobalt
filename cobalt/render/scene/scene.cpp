#include "scene.h"

namespace cobalt::render {

std::shared_ptr<Scene> Scene::create(const Scene::CreateInfo &createInfo) {
    std::shared_ptr<Scene> scene = std::shared_ptr<Scene>(new Scene(createInfo));

    return scene;
}

Scene::Scene(const CreateInfo &createInfo)
    : _camera{createInfo.camera}, _geometry{createInfo.geometry}, _components{createInfo.components} {
}

std::shared_ptr<Camera> Scene::camera() const {
    return _camera;
}

} // namespace cobalt::render
