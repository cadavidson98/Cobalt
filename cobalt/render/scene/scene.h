#ifndef CBLT_RENDER_SCENE_H
#define CBLT_RENDER_SCENE_H

#include "geometry/bounding_volume_scene_storage.h"
#include "render/component_storage.h"
#include "render/data/camera.h"

#include <memory>

namespace cblt::render {

class Scene {
public:
    struct CreateInfo {
        std::shared_ptr<Camera> camera;
        std::shared_ptr<geom::SceneStorage> geometry;
        std::shared_ptr<ComponentStorage> components;
    };

    static std::shared_ptr<Scene> create(const CreateInfo &createInfo);

    std::shared_ptr<Camera> camera() const;

    std::shared_ptr<geom::SceneStorage> storage() const {
        return _geometry;
    };

    std::shared_ptr<ComponentStorage> componentStorage() const {
        return _components;
    }

private:
    Scene(const CreateInfo &createInfo);
    Scene() = delete;

    std::shared_ptr<Camera> _camera;
    // fixme: need to change to spectrum std::shared_ptr<Texture> _environmentMap;
    std::shared_ptr<geom::SceneStorage> _geometry;
    std::shared_ptr<ComponentStorage> _components;
};

} // namespace cblt::render

#endif // CBLT_RENDER_SCENE_H
