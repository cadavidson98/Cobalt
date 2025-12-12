#include "scene_factory.h"

#include "byte_texture.h"
#include "component_storage.h"
#include "resolver.h"
#include "scene.h"
#include "texture.h"

#include "core/logging.h"
#include "geometry/bounding_volume_scene_storage.h"
#include "geometry/bounding_volume_types.h"
#include "geometry/mesh.h"
#include "geometry/sphere.h"
#include "io/exr_reader.h"
#include "io/mitsuba_reader.h"
#include "io/obj_reader.h"
#include "io/png_reader.h"
#include "math/math_types.h"

#include <cstring>
#include <filesystem>
#include <memory>

namespace cblt::render {

namespace {
class SceneDelegate final : public io::mitsuba::FileReaderDelegate {
public:
    SceneDelegate(std::filesystem::path rootDirectory)
        : _rootDirectory{rootDirectory}, _sceneGeometry{std::make_shared<geom::CoSceneStorage>()},
          _sceneComponents{std::make_shared<ComponentStorage>()} {
    }

    bool readSphere(const io::mitsuba::Shape<io::mitsuba::Sphere> &sphere) override {
        const geom::Primitive spherePrimitive = _sceneGeometry->addSphere(geom::CoSphere{
            .center = simd::vec3f(sphere.shape.center.x, sphere.shape.center.y, sphere.shape.center.z),
            .radius = sphere.shape.radius,
        });

        const io::mitsuba::Spectrum &spectrum = sphere.spectrum;
        _sceneComponents->addColor(
            spherePrimitive,
            CoColor{
                spectrum.red,
                spectrum.green,
                spectrum.blue,
            }
        );

        return true;
    }

    bool readMesh(const io::mitsuba::Shape<io::mitsuba::Mesh> &mesh) override {
        std::shared_ptr<geom::CoMesh> cobaltMesh;
        if (mesh.shape.fileExtension == "obj") {
            const std::filesystem::path filePath = _rootDirectory / mesh.shape.fileName;
            const std::optional<io::obj::Mesh> objMesh = io::obj::read(filePath.c_str());
            if (objMesh) {
                cobaltMesh = geom::CoMesh::create({
                    .positions = objMesh->positions,
                });
            }
        }

        if (!cobaltMesh) {
            return false;
        }

        const io::mitsuba::Spectrum &spectrum = mesh.spectrum;

        const geom::Primitive meshPrimitive = _sceneGeometry->addMesh(cobaltMesh);
        _sceneComponents->addColor(
            meshPrimitive,
            CoColor{
                spectrum.red,
                spectrum.green,
                spectrum.blue,
            }
        );

        return true;
    }

    bool readEmitter(const io::mitsuba::Emitter &emitter) override {
        if (_environmentMap) {
            CoLogInfo("Environment Map already loaded. Skipping.");
            return true;
        }

        std::shared_ptr<CoTexture> emissionMap = nullptr;
        if (emitter.emissionMap) {
            std::filesystem::path filePath = _rootDirectory / emitter.emissionMap.fileName;
            if (filePath.extension() == ".png") {
                const std::optional<io::png::ByteImage> image = io::png::read(filePath);
                if (!image) {
                    return false;
                }

                emissionMap = CoByteTexture::create({
                    .bytes = image->data,
                    .format = CoPixelFormat::Float,
                    .numChannels = image->channelCount,
                    .dimensions = image->size,
                });
            } else if (filePath.extension() == ".exr") {
                const std::optional<io::exr::Image> image = io::exr::read(filePath);
                if (!image) {
                    return false;
                }

                emissionMap = CoByteTexture::create({
                    .bytes = image->data,
                    .format = image->format == Imf::PixelType::HALF ? CoPixelFormat::Half : CoPixelFormat::Float,
                    .numChannels = image->channelCount,
                    .dimensions = image->size,
                });
            }

            if (!emissionMap) {
                return false;
            }
        }

        _environmentMap = emissionMap;
        return true;
    }

    bool readSensor(const io::mitsuba::Camera &sensor) override {
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

    std::shared_ptr<CoScene> buildScene() {
        const CoScene::CreateInfo createInfo{
            .camera = _camera,
            .environmentMap = _environmentMap,
            .geometry = _sceneGeometry,
            .components = _sceneComponents,
        };

        return CoScene::create(createInfo);
    }

private:
    std::filesystem::path _rootDirectory;

    std::shared_ptr<geom::CoSceneStorage> _sceneGeometry;
    std::shared_ptr<ComponentStorage> _sceneComponents;

    std::shared_ptr<CoCamera> _camera = {};
    std::shared_ptr<CoTexture> _environmentMap = {};
};

inline std::shared_ptr<CoScene> loadMitsubaScene(const CoSceneFactory::CreateInfo &createInfo) {
    std::shared_ptr<SceneDelegate> delegate = std::make_shared<SceneDelegate>(createInfo.fileName.parent_path());

    if (!io::mitsuba::read(createInfo.fileName.c_str(), delegate)) {
        return nullptr;
    }

    return delegate->buildScene();
}

} // anonymous namespace

std::shared_ptr<CoScene> CoSceneFactory::buildScene(const CoSceneFactory::CreateInfo &createInfo) {
    switch (createInfo.format) {
    case SceneFormat::kMitsuba : return loadMitsubaScene(createInfo);
    }

    return nullptr;
}

} // namespace cblt::render
