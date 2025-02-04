#include "scene_factory.h"

#include "logging.h"
#include "simd/simd_vec3.h"
#include "vec3.h"
#include "vec4.h"

#include "scene.h"

#include "private/obj_utilities.h"
#include "private/mitsuba_utilities.h"

#include <cstring>
#include <fstream>
#include <sstream>

namespace cblt::render {

std::shared_ptr<CoScene> CoSceneFactory::buildScene(const CoSceneFactory::CreateInfo &createInfo, core::CoCallback &callback) {
    switch(createInfo.format) {
        case SceneFormat::kMitsuba:
        return _loadMitsubaScene(createInfo, callback);
        case SceneFormat::kObj:
        default:
        return _loadObjScene(createInfo, callback);
    }
}

std::shared_ptr<CoScene> CoSceneFactory::_loadMitsubaScene(
    const CoSceneFactory::CreateInfo &createInfo,
    core::CoCallback &callback) {
    std::optional<utils::MitsubaScene> mitsubaScene =
        utils::readMitsuba(createInfo.fileName, *createInfo.parentDirectory, callback);
    if (!mitsubaScene) {
        return nullptr;
    }

    callback.pump("building scene", 100);

    CoScene::CreateFromDataInfo sceneInfo{
        .camera = std::move(mitsubaScene->camera),
        .mesh = std::move(mitsubaScene->meshes[0]),
        .environmentMap = std::move(mitsubaScene->environmentMap),
        .materials = nullptr,
        .ptexTextures = nullptr,
    };

    return CoScene::create(sceneInfo);
}

std::shared_ptr<CoScene> CoSceneFactory::_loadObjScene(
    const CoSceneFactory::CreateInfo &createInfo,
    core::CoCallback &callback) {
    std::shared_ptr<geom::CoMesh> objMesh = utils::readObjFile(createInfo.fileName);
    if (!objMesh) {
        return nullptr;
    }

    // it really should just return a dynamic array CoMesh(es)
    callback.pump("building scene", 50);
    CoScene::CreateFromDataInfo sceneCreateInfo {
        .camera = std::make_shared<CoCamera>(CoCamera::CreateFromProjectionInfo{}),
        .mesh = std::move(objMesh),
        .environmentMap = nullptr,
        .materials = nullptr,
        .ptexTextures = nullptr,
    };

    return CoScene::create(sceneCreateInfo);
}

} // namespace cblt::render
