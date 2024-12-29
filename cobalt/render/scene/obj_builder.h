#ifndef CBLT_RENDER_OBJ_BUILDER_H
#define CBLT_RENDER_OBJ_BUILDER_H

#include "scene_builder.h"

#include <fstream>

namespace cblt::geom {

class CoMesh;

} // namespace cblt::geom

namespace cblt::render {

class CoObjBuilder final : public CoSceneBuilder {
    public:
        CoObjBuilder(const CoSceneBuilder::CreateInfo &createInfo);

        bool buildMeshes() override;
        bool buildCameras() override;
        bool buildEnvironment() override;

        std::shared_ptr<CoScene> scene() const override;

    private:
        std::string _objFileName;

        std::shared_ptr<geom::CoMesh> _mesh;
};

} // namespace cblt::render

#endif // CBLT_RENDER_OBJ_BUILDER_H
