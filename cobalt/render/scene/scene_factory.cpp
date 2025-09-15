#include "scene_factory.h"

#include "image_reader.h"
#include "resolver.h"
#include "scene.h"
#include "texture.h"

#include "core/logging.h"
#include "geometry/bounding_volume_scene_storage.h"
#include "geometry/mesh.h"
#include "geometry/sphere.h"
#include "io/mitsuba_reader.h"
#include "io/obj_reader.h"
#include "math/math_types.h"

#include <cstring>
#include <filesystem>
#include <memory>

namespace cblt::render {

namespace {
class SceneDelegate final : public io::mitsuba::FileReaderDelegate {
public:
    SceneDelegate(std::filesystem::path rootDirectory)
        : _rootDirectory{rootDirectory}, _sceneGeometry{std::make_shared<geom::CoSceneStorage>()} {
    }

    bool readSphere(const io::mitsuba::Shape<io::mitsuba::Sphere> &sphere) override {
        _sceneGeometry->addSphere(geom::CoSphere{
            .center = simd::vec3f(sphere.shape.center.x, sphere.shape.center.y, sphere.shape.center.z),
            .radius = sphere.shape.radius,
        });

        return true;
    }

    bool readMesh(const io::mitsuba::Shape<io::mitsuba::Mesh> &mesh) override {
        std::shared_ptr<geom::CoMesh> cobaltMesh;
        if (mesh.shape.fileExtension == "obj") {
            const std::filesystem::path filePath = _rootDirectory / mesh.shape.fileName;
            const std::optional<io::obj::Mesh> objMesh = io::obj::read(filePath.c_str());
            if (objMesh) {
                cobaltMesh = geom::CoMesh::create({
                    .positions =
                        {
                                    .vertices = objMesh->positions.vertices,
                                    .vertexCount = objMesh->positions.vertexCount,
                                    .triangleIndices = objMesh->positions.triangleIndices,
                                    .triangleCount = objMesh->positions.triangleCount,
                                    .patchIndices = objMesh->positions.patchIndices,
                                    .patchCount = objMesh->positions.patchCount,
                                    },
                });
            }
        }

        if (!cobaltMesh) {
            return false;
        }

        _sceneGeometry->addMesh(cobaltMesh);
        _spectrums.push_back(mesh.spectrum);
        // 2 different options here
        // 1) we PROMOTE the reorder to here, so we properly reorder the spectrums
        /*
            a) need a "structure of arrays" - actually not, this is going to devolve to the original problem we had w/
           union b) need to "recursively reorder" i) we need to "Promote" the "Entity" & "Component" to Core - not a
           crime, not a smell This would "suggest" the IntersectionResult Signature is actually: struct
           IntersectionResult { float hitTime = std::numeric_limits<float>::max(); Entity hitEntity; Primitive geometry;
                    };

                ii) Embrace the "typed index" - same size as a raw ptr (and less than a shared), but no semantics (only
           const) struct Primitive { PrimitiveType type; uint32_t idx;
                };

                std::zip_view<StringHash, BasicRendererEntity> entities = builder->makeSlice();
                std::span<const Primitives> primitives = storage->primitives();
                // explicitly sorted
                geom::BoundingVolume::Sorter sorter;
                std::span<const MortonPrimitive> mortonEncodedPrimtives = sorter->encodeAndSort(storage);
                // reorder primitives
                for (const Primitive &primitive : std::member_view<const Primitive>(mortonEncodedPrimitives)) {

                }
                geom::BoundingVolume<geom::SceneStorage>(storage, mortonEncodedPrimitives);
        */

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
            emissionMap = readImage({
                .fileName = filePath.c_str(),
            });

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
        };

        return CoScene::create(createInfo);
    }

private:
    std::filesystem::path _rootDirectory;

    std::shared_ptr<geom::CoSceneStorage> _sceneGeometry;
    std::vector<io::mitsuba::Spectrum> _spectrums;

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
