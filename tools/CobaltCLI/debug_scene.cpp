#include "debug_scene.h"

#include "assets.h"
#include "scene.h"
#include "scene_factory.h"
#include "texture.h"

#include <optional>
#include <string>

namespace cblt::tools {

std::shared_ptr<render::CoScene> debugScene(core::CoCallback &callback) {
    static const std::string teapotMesh = asset::kAssetsMeshesDir + "teapot/teapot.obj";

    return render::CoSceneFactory::buildScene(
        {
            .fileName = teapotMesh,
            .parentDirectory = std::optional<std::string>(asset::kAssetsMeshesDir + std::string("/teapot")),
            .format = render::CoSceneFactory::SceneFormat::kObj,
        },
        callback
    );

    // Ptex::PtexTexture *meshTexture = _textures->get(teapotTexture.c_str(), errorString);

    // std::shared_ptr<render::CoTexture> environmentMap = cblt::render::CoTexture::create({
    //     .fileName = cblt::tools::asset::kAssetsTexturesDir + "arches.exr",
    //     .fileExtension = "exr",
    // });
}

} // namespace cblt::tools
