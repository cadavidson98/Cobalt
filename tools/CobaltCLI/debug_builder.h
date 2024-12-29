#ifndef CBLT_RENDER_SCENE_DEBUG_BUILDER_H
#define CBLT_RENDER_SCENE_DEBUG_BUILDER_H

#include "scene_builder.h"

namespace cblt {

namespace geom {

class CoMesh;

} // namespace geom

namespace render {

class CoTexture;
class CoCamera;

} // namespace render

} // namespace cblt

namespace cblt::tools {

class CoDebugBuilder final : render::CoSceneBuilder {
    public:
        CoDebugBuilder();

        bool buildMeshes() override;
        bool buildCameras() override;
        bool buildEnvironment() override;

        std::shared_ptr<render::CoScene> scene() const override;

    private:
        std::shared_ptr<geom::CoMesh> _mesh;
        std::shared_ptr<render::CoTexture> _environmentMap;
        std::shared_ptr<render::CoCamera> _camera;
};

} // namespace cblt::tools

#endif // CBLT_RENDER_SCENE_DEBUG_RENDER_H
