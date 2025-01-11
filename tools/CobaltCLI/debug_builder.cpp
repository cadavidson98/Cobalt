#include "debug_builder.h"

#include "assets.h"
#include "scene.h"
#include "texture.h"

namespace cblt::tools {

CoDebugBuilder::CoDebugBuilder() {
    static constexpr size_t kMaxTextureMemory = 1024 * 1024 * 4;  // 4 mB 
    _textures = Ptex::PtexCache::create(20, kMaxTextureMemory, true);
}

CoDebugBuilder::~CoDebugBuilder() {
    _textures->release();
}

bool CoDebugBuilder::buildMeshes() {
    const std::optional<MeshBuffers> meshBuffers = _readObjFile(asset::kAssetsMeshesDir + "teapot/teapot.obj");
    if (!meshBuffers) {
        return false;
    }

    std::shared_ptr<cblt::geom::CoMesh> teapotMesh = cblt::geom::CoMesh::create(
        {
            .positions = meshBuffers->positions,
            .numVertices = meshBuffers->numPositions,
            .indices = meshBuffers->indices,
            .numIndices = meshBuffers->numIndices,
        }
    );

    if (!teapotMesh) {
        return false;
    }

    Ptex::String errorString;
    static const std::string teapotTexture = asset::kAssetsMeshesDir + "teapot/teapot.ptx";
    Ptex::PtexTexture *meshTexture = _textures->get(teapotTexture.c_str(), errorString);

    if (!meshTexture) {
        return false;
    }

    const Ptex::PtexTexture::Info textureInfo = meshTexture->getInfo();

    _mesh = teapotMesh;
    return true;
}

bool CoDebugBuilder::buildCameras() {
    static const cblt::render::CoCamera kDefaultCamera(cblt::render::CoCamera::CreateFromProjectionInfo{
        .hFov = cblt::toRadians(40.f),
        .vFov = cblt::toRadians(40.f),
        .filmSize = cblt::CoSize(2.2f, 2.2f),
        .cameraToWorld = cblt::utils::translationMatrix({0.f, 0.f, -5.f}),
    });

    _camera = std::shared_ptr<render::CoCamera>(new render::CoCamera(kDefaultCamera));
    return _camera != nullptr;
}

bool CoDebugBuilder::buildEnvironment() {
    _environmentMap = cblt::render::CoTexture::create({
        .fileName = cblt::tools::asset::kAssetsTexturesDir + "arches.exr",
        .fileExtension = "exr",
    });

    return _environmentMap != nullptr;
}

std::shared_ptr<render::CoScene> CoDebugBuilder::scene() const {
    if (!_camera || !_mesh || !_environmentMap) {
        return nullptr;
    }

    return render::CoScene::Create(render::CoScene::CreateFromDataInfo{
        .camera = *_camera,
        .mesh = _mesh,
        .environmentMap = _environmentMap,
    });
}

} // namespace cblt::tools
