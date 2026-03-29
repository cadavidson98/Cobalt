#ifndef COBALT_RENDER_SCENE_H
#define COBALT_RENDER_SCENE_H

#include "geometry/bounding_volume_scene_storage.h"
#include "render/component_storage.h"
#include "render/data/camera.h"
#include "render/data/texture.h"

#include <memory>

namespace cobalt::render {

class Scene {
public:
    struct CreateInfo {
        std::shared_ptr<Camera> camera;
        std::shared_ptr<Texture> environmentMap;
        std::shared_ptr<geom::SceneStorage> geometry;
        std::shared_ptr<ComponentStorage> components;
    };

    static std::shared_ptr<Scene> create(const CreateInfo &createInfo);

    std::shared_ptr<Camera> camera() const;

    std::shared_ptr<Texture> environmentMap() const;

    std::shared_ptr<geom::SceneStorage> storage() const;

    std::shared_ptr<ComponentStorage> componentStorage() const;

private:
    Scene(const CreateInfo &createInfo);
    Scene() = delete;

    std::shared_ptr<Camera> _camera;
    std::shared_ptr<Texture> _environmentMap;
    std::shared_ptr<geom::SceneStorage> _geometry;
    std::shared_ptr<ComponentStorage> _components;
};

} // namespace cobalt::render

#endif // COBALT_RENDER_SCENE_H
