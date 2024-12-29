#include "obj_builder.h"

#include "camera.h"
#include "math_utils.h"
#include "scene.h"

#include <cassert>

namespace cblt::render {

CoObjBuilder::CoObjBuilder(const CreateInfo &createInfo): _objFileName{createInfo.fileName} {
}

bool CoObjBuilder::buildMeshes() {
    std::optional<MeshBuffers> objBuffers = _readObjFile(_objFileName);
    if (!objBuffers) {
        return false;
    }

    _mesh = geom::CoMesh::create({

    });

    return _mesh != nullptr;
}

bool CoObjBuilder::buildCameras() {
    return true;
}

bool CoObjBuilder::buildEnvironment() {
    return true;
}

std::shared_ptr<CoScene> CoObjBuilder::scene() const {
    static const CoCamera::CreateFromProjectionInfo kDefaultCamera{
        .hFov = toRadians(30.f),
        .vFov = toRadians(30.f),
        .filmSize = {2.2f, 2.2f},
        .cameraToWorld = utils::translationMatrix({0.f, 0.f, -5.f}),
    };

    return CoScene::Create({
        .camera = CoCamera(kDefaultCamera),
        .mesh = _mesh,
        .environmentMap = nullptr,
    });
}

} // namespace cblt::render
