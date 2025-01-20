#ifndef CBLT_RENDER_SCENE_DEBUG_BUILDER_H
#define CBLT_RENDER_SCENE_DEBUG_BUILDER_H

#include "scene_builder.h"
#include "dynamic_array.h"

#include "Ptexture.h"

namespace cblt {

namespace geom {

class CoMesh;

} // namespace geom

namespace render {

class CoTexture;
class CoCamera;
class CoMaterial;

} // namespace render

} // namespace cblt

namespace cblt::tools {

class CoDebugBuilder final : render::CoSceneBuilder {
    public:
        CoDebugBuilder();
        virtual ~CoDebugBuilder();
        virtual bool buildMeshes(std::function<void(int, const char *)> callback) override;
        virtual bool buildCameras(std::function<void(int, const char *)> callback) override;
        virtual bool buildEnvironment(std::function<void(int, const char *)> callback) override;

        std::shared_ptr<render::CoScene> scene() override;

    private:
        std::shared_ptr<geom::CoMesh> _mesh;
        std::shared_ptr<render::CoTexture> _environmentMap;
        std::shared_ptr<render::CoCamera> _camera;
        Ptex::PtexCache *_textures;
        CoDynamicArray<render::CoMaterial> _materials;
};

} // namespace cblt::tools

#endif // CBLT_RENDER_SCENE_DEBUG_RENDER_H
