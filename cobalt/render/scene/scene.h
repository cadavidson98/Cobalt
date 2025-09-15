#ifndef CBLT_RENDER_SCENE_H
#define CBLT_RENDER_SCENE_H

#include "geometry/bounding_volume_scene_storage.h"
#include "render/data/camera.h"
#include "render/data/texture.h"

#include <memory>

namespace cblt::render {

class CoScene {
public:
    struct CreateInfo {
        std::shared_ptr<CoCamera> camera;
        std::shared_ptr<CoTexture> environmentMap;
        std::shared_ptr<geom::CoSceneStorage> geometry;
    };

    static std::shared_ptr<CoScene> create(const CreateInfo &createInfo);

    std::shared_ptr<CoCamera> camera() const;

    std::shared_ptr<geom::CoSceneStorage> storage() const {
        return _geometry;
    };

private:
    CoScene(const CreateInfo &createInfo);
    CoScene() = delete;

    std::shared_ptr<CoCamera> _camera;
    std::shared_ptr<CoTexture> _environmentMap;
    std::shared_ptr<geom::CoSceneStorage> _geometry;
};

} // namespace cblt::render

#endif // CBLT_RENDER_SCENE_H
