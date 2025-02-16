#include "scene_factory.h"

#include "scene.h"
#include "texture.h"

#include "core/logging.h"
#include "math/simd/simd_vec3.h"
#include "math/vec3.h"
#include "math/vec4.h"
#include "private/mitsuba_utilities.h"
#include "private/obj_utilities.h"

#include <cstring>
#include <fstream>
#include <unordered_map>

namespace cblt::render {

std::shared_ptr<CoScene>
CoSceneFactory::buildScene(const CoSceneFactory::CreateInfo &createInfo, core::CoCallback &callback) {
    switch (createInfo.format) {
    case SceneFormat::kMitsuba : return _loadMitsubaScene(createInfo, callback);
    }

    return nullptr;
}

std::shared_ptr<CoScene>
CoSceneFactory::_loadMitsubaScene(const CoSceneFactory::CreateInfo &createInfo, core::CoCallback &callback) {
    callback.pump("Parsing scene file");
    std::optional<utils::MitsubaScene> mitsubaScene =
        utils::readMitsuba(createInfo.fileName, *createInfo.parentDirectory);
    if (!mitsubaScene) {
        return nullptr;
    }

    callback.pump("building scene", 100);

    std::shared_ptr<CoCamera> camera = std::shared_ptr<CoCamera>(new CoCamera({
        .hFov = mitsubaScene->camera.fov,
        .vFov = mitsubaScene->camera.fov,
        .cameraToWorld = mitsubaScene->camera.transform,
    }));

    Ptex::PtexCache *ptexCache = nullptr;
    uint64_t materialIdx = 0;
    std::unordered_map<std::shared_ptr<utils::MitsubaBSDF>, uint64_t> materialMap = {};

    std::shared_ptr<CoTexture> environmentMap = nullptr;
    if (mitsubaScene->environmentMap) {
        environmentMap = CoTexture::create({
            .fileName = mitsubaScene->environmentMap.fileName,
            .fileExtension = mitsubaScene->environmentMap.fileExtension,
        });
    }

    for (const std::shared_ptr<utils::MitsubaBSDF> &bsdf : mitsubaScene->materials) {
        // make material
        // if (material.texture && material.texture.fileExtension == "ptex") {
        //     if (!ptexCache) {
        //         ptexCache = Ptex::PtexCache::create();
        //     }
        //     Ptex::String errorString;
        //     Ptex::PtexTexture *texture = ptexCache->get(material.texture.fileName.c_str(), errorString);
        // }

        const utils::MitsubaBSDF::Properties principledParams = bsdf->properties();

        // materialMap.emplace(&material, ++materialIdx);
    }

    // make meshes
    uint32_t meshIdx = 0;
    CoDynamicArray<CoScene::GeometryComponent> meshes(mitsubaScene->meshes.size());
    for (const utils::MitsubaMesh &mesh : mitsubaScene->meshes) {
        // load geometry
        std::shared_ptr<geom::CoMesh> instanceMesh = nullptr;
        if (mesh.fileExtension == "obj") {
            instanceMesh = utils::readObjFile(mesh.fileName);
            if (!instanceMesh) {
                return nullptr;
            }
        }
        // get material

        // make instance
        meshes[++meshIdx] = CoScene::GeometryComponent{
            .mesh = instanceMesh,
            .transform = mesh.transform,
        };
    }

    CoScene::CreateFromDataInfo sceneInfo{
        .camera = std::move(camera),
        .environmentMap = std::move(environmentMap),
        .meshes = std::move(meshes),
        .materials = nullptr,
        .ptexTextures = ptexCache,
    };

    return CoScene::create(sceneInfo);
}

} // namespace cblt::render
