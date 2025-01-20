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

bool CoDebugBuilder::buildMeshes(std::function<void(int, const char *)> progressCallback) {
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

    progressCallback(25, "loading meshes");

    Ptex::String errorString;
    static const std::string teapotTexture = asset::kAssetsMeshesDir + "teapot/teapot.ptx";
    Ptex::PtexTexture *meshTexture = _textures->get(teapotTexture.c_str(), errorString);

    if (!meshTexture) {
        return false;
    }

    const Ptex::PtexTexture::Info textureInfo = meshTexture->getInfo();
    progressCallback(25, "loading materials");

    _materials = CoDynamicArray<render::CoMaterial>(1);
    _materials[0] = render::CoMaterial(
        render::CoSurfaceParams{
            .baseColor = vec4f(1.f, 0.f, 0.f, 1.f),
        },
        meshTexture
    );

    _mesh = teapotMesh;
    return true;
}

bool CoDebugBuilder::buildCameras(std::function<void(int, const char *)> progressCallback) {
    static const cblt::render::CoCamera kDefaultCamera(cblt::render::CoCamera::CreateFromProjectionInfo{
        .hFov = cblt::toRadians(40.f),
        .vFov = cblt::toRadians(40.f),
        .filmSize = vec2f{2.2f, 2.2f},
        .cameraToWorld = cblt::utils::translationMatrix({0.f, 0.f, -5.f}),
    });

    progressCallback(25, "loading camera frames");
    _camera = std::shared_ptr<render::CoCamera>(new render::CoCamera(kDefaultCamera));
    return _camera != nullptr;
}

bool CoDebugBuilder::buildEnvironment(std::function<void(int, const char *)> progressCallback) {
    _environmentMap = cblt::render::CoTexture::create({
        .fileName = cblt::tools::asset::kAssetsTexturesDir + "arches.exr",
        .fileExtension = "exr",
    });

    progressCallback(25, "loading environments");
    return _environmentMap != nullptr;
}

std::shared_ptr<render::CoScene> CoDebugBuilder::scene() {
    if (!_camera || !_mesh || !_environmentMap || !_materials) {
        return nullptr;
    }

    render::CoScene::CreateFromDataInfo createInfo{
        .camera = *_camera,
        .mesh = _mesh,
        .environmentMap = _environmentMap,
        .materials = std::move(_materials),
        .ptexTextures = _textures,
    };

    return render::CoScene::create(createInfo);
}

} // namespace cblt::tools
