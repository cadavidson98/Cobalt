#include "scene_factory.h"

#include "image_reader.h"
#include "resolver.h"
#include "scene.h"
#include "texture.h"

#include "core/logging.h"
#include "core/string_utilities.h"
#include "math/math_types.h"
#include "private/mitsuba_utilities.h"
#include "private/obj_utilities.h"

#include <cstring>
#include <unordered_map>

namespace cblt::render {

class CoSceneFactoryDelegate final : public utils::MitsubaDelegate {
public:
    bool readMesh(const utils::MitsubaMesh &mesh) override {
        std::shared_ptr<geom::CoMesh> cobaltMesh;
        if (mesh.fileExtension == "obj") {
            cobaltMesh = utils::readObjFile(core::appendFileToPath(_rootDirectory, mesh.fileName));
        }

        if (!cobaltMesh) {
            return false;
        }

        CoUUID materialID = CoScene::kInvalidID;

        if (std::holds_alternative<std::string>(mesh.material)) {
            auto materialIter = _materialMap.find(std::get<std::string>(mesh.material));
            if (materialIter == _materialMap.end()) {
                return false;
            }

            materialID = materialIter->second;
        } else {
            if (!readBsdf(std::get<std::shared_ptr<utils::MitsubaBSDF>>(mesh.material))) {
                return false;
            }
            materialID = _lastMaterialID;
        }

        const CoUUID geometryID = _geometry.size();

        _geometry.push_back(CoScene::GeometryComponent{
            .mesh = cobaltMesh,
            .transform = mesh.transform,
        });

        _primitives.push_back(CoScene::Primitive{
            .geometryIdx = geometryID,
            .materialIdx = materialID,
        });
        return true;
    }

    bool readBsdf(std::shared_ptr<utils::MitsubaBSDF> bsdf) override {
        auto makeFloatNode = [](const std::variant<float, utils::MitsubaTexture> &value) -> CoMaterialNode<float> {
            if (std::holds_alternative<float>(value)) {
                return CoMaterialNode(std::get<float>(value));
            }

            throw std::runtime_error("Texture not yet supported");
            // TODO: cache texture
        };

        auto makeSpectrumNode = [](const std::variant<CoSpectrum, utils::MitsubaTexture> &value
                                ) -> CoMaterialNode<CoSpectrum> {
            if (std::holds_alternative<CoSpectrum>(value)) {
                return CoMaterialNode(std::get<CoSpectrum>(value));
            }

            throw std::runtime_error("Texture not yet supported");
            // TODO: cache texture
        };

        const utils::MitsubaBSDF::Properties principledParameters = bsdf->properties();

        const CoMaterial::Properties materialProperties{
            .baseColor = makeSpectrumNode(principledParameters.baseColor),
            .metallic = makeFloatNode(principledParameters.metallic),
            .subsurface = makeFloatNode(principledParameters.subsurface),
            .ior = makeFloatNode(principledParameters.ior),
            .specular = makeFloatNode(principledParameters.specular),
            .specularTint = makeFloatNode(principledParameters.specularTint),
            .specularTransmission = makeFloatNode(principledParameters.specularTransmission),
            .roughness = makeFloatNode(principledParameters.roughness),
            .anisotropic = makeFloatNode(principledParameters.anisotropic),
            .sheen = makeFloatNode(principledParameters.sheen),
            .sheenTint = makeFloatNode(principledParameters.sheenTint),
            .clearcoat = makeFloatNode(principledParameters.clearcoat),
            .clearcoatGloss = makeFloatNode(principledParameters.clearcoatGloss),
        };

        _lastMaterialID = _materials.size();

        _materials.push_back({
            .surface = std::shared_ptr<CoMaterial>(new CoMaterial(materialProperties)),
        });

        const std::string materialID = bsdf->referenceID();
        if (materialID.length()) {
            _materialMap.emplace(materialID, _lastMaterialID);
        }

        return true;
    }

    bool readEmitter(const utils::MitsubaEmitter &emitter) override {
        if (_environmentMap) {
            CoLogInfo("Environment Map already loaded. Skipping.");
            return true;
        }

        std::shared_ptr<CoTexture> emissionMap = nullptr;
        if (emitter.emissionMap) {
            emissionMap = readImage({
                .fileName = core::appendFileToPath(_rootDirectory, emitter.emissionMap.fileName),
            });

            if (!emissionMap) {
                return false;
            }
        }

        _environmentMap = emissionMap;
        return true;
    }

    bool readSensor(const utils::MitsubaCamera &sensor) override {
        if (_camera) {
            CoLogInfo("Camera already loaded. Skipping.");
            return true;
        }

        const CoCamera::CreateFromProjectionInfo cameraInfo{
            .hFov = sensor.fov,
            .vFov = sensor.fov,
            .cameraToWorld = sensor.transform,
        };

        _camera = std::make_shared<CoCamera>(cameraInfo);
        return true;
    }

    CoSceneFactoryDelegate(core::CoCallback &callback, std::string_view rootDirectory)
        : _progressCallback{callback}, _rootDirectory{rootDirectory} {
    }

    std::shared_ptr<CoScene> scene() {
        CoScene::CreateInfo createInfo{
            .camera = _camera,
            .environmentMap = _environmentMap,
            .primitives = _primitives,
            .meshes = _geometry,
            .materials = _materials,
        };

        return CoScene::create(createInfo);
    }

private:
    std::reference_wrapper<core::CoCallback> _progressCallback;
    std::string _rootDirectory;

    CoUUID _lastMaterialID = CoScene::kInvalidID;

    std::vector<CoScene::Primitive> _primitives = {};
    std::vector<CoScene::GeometryComponent> _geometry = {};
    std::vector<CoScene::MaterialComponent> _materials = {};

    std::shared_ptr<CoCamera> _camera = {};
    std::shared_ptr<CoTexture> _environmentMap = {};

    std::unordered_map<std::string, CoUUID> _materialMap = {};
};

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
    std::shared_ptr<CoSceneFactoryDelegate> delegate =
        std::make_shared<CoSceneFactoryDelegate>(callback, createInfo.parentDirectory.value_or(""));

    if (!utils::readMitsuba(createInfo.fileName, delegate)) {
        return nullptr;
    }

    callback.pump("building scene", 100);

    return delegate->scene();
}

} // namespace cblt::render
