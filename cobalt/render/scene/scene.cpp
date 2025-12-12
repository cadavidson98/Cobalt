#include "scene.h"

namespace cblt::render {

std::shared_ptr<CoScene> CoScene::create(const CoScene::CreateInfo &createInfo) {
    std::shared_ptr<CoScene> scene = std::shared_ptr<CoScene>(new CoScene(createInfo));

    return scene;
}

CoScene::CoScene(const CreateInfo &createInfo)
    : _camera{createInfo.camera}, _environmentMap{createInfo.environmentMap}, _geometry{createInfo.geometry},
      _components{createInfo.components} {
}

std::shared_ptr<CoCamera> CoScene::camera() const {
    return _camera;
}

} // namespace cblt::render
