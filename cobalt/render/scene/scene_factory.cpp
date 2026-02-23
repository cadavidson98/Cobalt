#include "scene_factory.h"

#include "component_storage.h"
#include "scene.h"

#include "color/polynomial_spectrum.h"
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

namespace cobalt::render {

namespace {
class SceneDelegate final : public io::mitsuba::FileReaderDelegate {
public:
    SceneDelegate(std::filesystem::path rootDirectory)
        : _rootDirectory{rootDirectory}, _sceneGeometry{std::make_shared<geom::SceneStorage>()},
          _sceneComponents{std::make_shared<ComponentStorage>()} {
    }

    bool readSphere(const io::mitsuba::Shape<io::mitsuba::Sphere> &sphere) override {
        const geom::Primitive spherePrimitive = _sceneGeometry->addSphere(
            geom::Sphere{
                .center = simd::vec3f(sphere.shape.center.x, sphere.shape.center.y, sphere.shape.center.z),
                .radius = sphere.shape.radius,
            }
        );

        const io::mitsuba::Spectrum &spectrum = sphere.spectrum;
        _sceneComponents->addSpectrum(
            spherePrimitive,
            color::PolynomialSpectrum{
                .coefficients = spectrum.coefficients,
            }
        );

        return true;
    }

    bool readMesh(const io::mitsuba::Shape<io::mitsuba::Mesh> &mesh) override {
        std::shared_ptr<geom::Mesh> cobaltMesh;
        if (mesh.shape.fileExtension == "obj") {
            const std::filesystem::path filePath = _rootDirectory / mesh.shape.fileName;
            const std::optional<io::obj::Mesh> objMesh = io::obj::read(filePath.c_str());
            if (objMesh) {
                cobaltMesh = geom::Mesh::create({
                    .positions = objMesh->positions,
                });
            }
        }

        if (!cobaltMesh) {
            return false;
        }

        const io::mitsuba::Spectrum &spectrum = mesh.spectrum;

        const geom::Primitive meshPrimitive = _sceneGeometry->addMesh(cobaltMesh);
        _sceneComponents->addSpectrum(
            meshPrimitive,
            color::PolynomialSpectrum{
                .coefficients = spectrum.coefficients,
            }
        );

        return true;
    }

    bool readEmitter(const io::mitsuba::Emitter &emitter) override {
        if (emitter.emissionMap) {
            std::filesystem::path filePath = _rootDirectory / emitter.emissionMap.fileName;
            if (filePath.extension() == ".png") {
                const std::optional<io::png::ByteImage> image = io::png::read(filePath);
                if (!image) {
                    return false;
                }

            } else if (filePath.extension() == ".exr") {
                const std::optional<io::exr::Image> image = io::exr::read(filePath);
                if (!image) {
                    return false;
                }
            }
        }

        return true;
    }

    bool readSensor(const io::mitsuba::Camera &sensor) override {
        if (_camera) {
            CoLogInfo("Camera already loaded. Skipping.");
            return true;
        }

        const Camera::CreateFromProjectionInfo cameraInfo{
            .hFov = sensor.fov,
            .vFov = sensor.fov,
            .cameraToWorld = sensor.transform,
        };

        _camera = std::make_shared<Camera>(cameraInfo);
        return true;
    }

    std::shared_ptr<Scene> buildScene() {
        const Scene::CreateInfo createInfo{
            .camera = _camera,
            .geometry = _sceneGeometry,
            .components = _sceneComponents,
        };

        return Scene::create(createInfo);
    }

private:
    std::filesystem::path _rootDirectory;

    std::shared_ptr<geom::SceneStorage> _sceneGeometry;
    std::shared_ptr<ComponentStorage> _sceneComponents;

    std::shared_ptr<Camera> _camera = {};
};

inline std::shared_ptr<Scene> loadMitsubaScene(const SceneFactory::CreateInfo &createInfo) {
    std::shared_ptr<SceneDelegate> delegate = std::make_shared<SceneDelegate>(createInfo.fileName.parent_path());

    if (!io::mitsuba::read(createInfo.fileName.c_str(), delegate)) {
        return nullptr;
    }

    return delegate->buildScene();
}

} // anonymous namespace

std::shared_ptr<Scene> SceneFactory::buildScene(const SceneFactory::CreateInfo &createInfo) {
    switch (createInfo.format) {
    case SceneFormat::kMitsuba :
        return loadMitsubaScene(createInfo);
    }

    return nullptr;
}

} // namespace cobalt::render
