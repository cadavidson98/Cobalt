#include "scene_builder.h"

#include "component_storage.h"
#include "scene.h"
#include "texture.h"

#include "color/pixel_buffer.h"
#include "color/polynomial_spectrum.h"
#include "color/rgb.h"
#include "color/xyz.h"
#include "core/logging.h"
#include "geometry/bounding_volume_scene_storage.h"
#include "geometry/bounding_volume_types.h"
#include "geometry/mesh.h"
#include "geometry/sphere.h"
#include "io/image/exr.h"
#include "io/image/image.h"
#include "io/image/png.h"
#include "io/mitsuba_reader.h"
#include "io/obj_reader.h"
#include "math/math_types.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <memory>

namespace cobalt::render {

namespace {

constexpr float kEpsilon = 1e-6f;

template<color::rgb::Colorspace colorspace>
struct image_traits {
    using colorspace_traits = color::rgb::colorspace_traits<colorspace>;

    static constexpr io::image::ColorSpace kColorspace = {
        .whitePoint = color::xyz::chromaticity(colorspace_traits::kWhitePoint),
        .red = color::xyz::chromaticity(colorspace_traits::kRed),
        .green = color::xyz::chromaticity(colorspace_traits::kGreen),
        .blue = color::xyz::chromaticity(colorspace_traits::kBlue),
    };
};

struct ColorspaceData {
    color::rgb::Colorspace colorspace;
    io::image::ColorSpace metadata;
};

template<size_t idx>
constexpr void makeColorspaceMetadata(std::span<ColorspaceData> metadatas) {
    metadatas[idx].colorspace = color::rgb::kAllColorspaces[idx];
    metadatas[idx].metadata = image_traits<color::rgb::kAllColorspaces[idx]>::kColorspace;

    makeColorspaceMetadata<idx + 1>(metadatas);
}

template<>
constexpr void makeColorspaceMetadata<std::size(color::rgb::kAllColorspaces)>(
    [[maybe_unused]] std::span<ColorspaceData> metadatas
) {
    return;
}

consteval std::array<ColorspaceData, color::rgb::kColorspaceCount> makeColorspaceMetadata() {
    std::array<ColorspaceData, color::rgb::kColorspaceCount> data;
    makeColorspaceMetadata<0>(data);
    return data;
}

bool approximatelyEqual(const io::image::ColorSpace &imageColorspace, const io::image::ColorSpace referenceColorspace) {
    return lengthSqr(abs(imageColorspace.whitePoint - referenceColorspace.whitePoint)) < kEpsilon &&
           lengthSqr(abs(imageColorspace.red - referenceColorspace.red)) < kEpsilon &&
           lengthSqr(abs(imageColorspace.green - referenceColorspace.green)) < kEpsilon &&
           lengthSqr(abs(imageColorspace.blue - referenceColorspace.blue)) < kEpsilon;
}

class ImageDelegate final : public io::image::FileReaderDelegate {
public:
    virtual ~ImageDelegate() = default;

    [[nodiscard]] bool readMetadata(const io::image::Metadata &metadata) override {
        if (_pixelBuffer) {
            CoLogError("PixelBuffer already allocated");
            return false;
        }

        static constexpr std::array<ColorspaceData, color::rgb::kColorspaceCount> data = makeColorspaceMetadata();

        auto validColorspace = std::ranges::find_if(data, [metadata](ColorspaceData bar) -> bool {
            return approximatelyEqual(metadata.colorspace, bar.metadata);
        });

        if (validColorspace == data.end()) {
            CoLogError("Unrecognized color space in image");
            return false;
        }

        // TODO: transfer function (gamma) handling

        _pixelBuffer = color::PixelBuffer::create({
            .colorspace = validColorspace->colorspace,
            .size = metadata.size,
        });

        _pixelStride = metadata.channelCount;

        return bool(_pixelBuffer);
    }

    [[nodiscard]] bool readRow(size_t row, std::span<const float> values) override {
        if (!_pixelBuffer) {
            CoLogError("PixelBuffer not allocated yet");
            return false;
        }

        const size_t width = _pixelBuffer->size().x;

        if (values.size() != width * _pixelStride) {
            CoLogError("Size mismatch: PixelBuffer with row %lu, actual %zu", width, values.size());
            return false;
        }

        std::span<color::rgb::Value> rowColors = _pixelBuffer->scanline(row);
        for (size_t idx = 0; idx < width; ++idx) {
            // TODO: Naively assume layout is always RGB
            rowColors[idx] = {
                .r = values[_pixelStride * idx + 0],
                .g = values[_pixelStride * idx + 1],
                .b = values[_pixelStride * idx + 2],
            };
        }

        return true;
    }

    std::shared_ptr<color::PixelBuffer> pixelBuffer() {
        return _pixelBuffer;
    }

private:
    std::shared_ptr<color::PixelBuffer> _pixelBuffer = {};

    size_t _pixelStride = 0;
};

class SceneDelegate final : public io::mitsuba::FileReaderDelegate {
public:
    SceneDelegate(std::filesystem::path rootDirectory)
        : _rootDirectory{rootDirectory}, _sceneGeometry{std::make_shared<geom::SceneStorage>()},
          _sceneComponents{std::make_shared<ComponentStorage>()} {
    }

    virtual ~SceneDelegate() = default;

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
            const std::filesystem::path filePath = _rootDirectory / emitter.emissionMap.fileName;
            _environmentMap = readTexture(filePath);
        }

        return true;
    }

    bool readSensor(const io::mitsuba::Camera &sensor) override {
        if (_camera) {
            CoLogInfo("Camera already loaded. Skipping.");
            return true;
        }

        const Camera::CreateInfo cameraInfo{
            .hFov = sensor.fov,
            .vFov = sensor.fov,
            .cameraToWorld = sensor.transform,
        };

        _camera = std::make_shared<Camera>(cameraInfo);
        return true;
    }

    std::shared_ptr<Texture> readTexture(std::filesystem::path texturePath) {
        std::shared_ptr<color::PixelBuffer> pixelBuffer;
        if (texturePath.extension() == ".png") {
            ImageDelegate imageDelegate;
            const bool readImage = io::image::png::read(texturePath, imageDelegate);
            if (!readImage) {
                return nullptr;
            }

            pixelBuffer = imageDelegate.pixelBuffer();
        } else if (texturePath.extension() == ".exr") {
            ImageDelegate imageDelegate;
            const bool readImage = io::image::exr::read(texturePath, imageDelegate);
            if (!readImage) {
                return nullptr;
            }

            pixelBuffer = imageDelegate.pixelBuffer();
        } else {
            CoLogError("Unsupported file type: %s", texturePath.extension().c_str());
            return nullptr;
        }

        return render::Texture::create({.pixelBuffer = pixelBuffer});
    };

    std::shared_ptr<Scene> buildScene() {
        const Scene::CreateInfo createInfo{
            .camera = _camera,
            .environmentMap = _environmentMap,
            .geometry = _sceneGeometry,
            .components = _sceneComponents,
        };

        return Scene::create(createInfo);
    }

private:
    std::filesystem::path _rootDirectory;

    std::shared_ptr<render::Texture> _environmentMap;

    std::shared_ptr<geom::SceneStorage> _sceneGeometry;
    std::shared_ptr<ComponentStorage> _sceneComponents;

    std::shared_ptr<Camera> _camera = {};
};

inline std::shared_ptr<Scene> loadMitsubaScene(const SceneBuilder::CreateInfo &createInfo) {
    std::shared_ptr<SceneDelegate> delegate = std::make_shared<SceneDelegate>(createInfo.fileName.parent_path());

    if (!io::mitsuba::read(createInfo.fileName.c_str(), delegate)) {
        return nullptr;
    }

    return delegate->buildScene();
}

} // anonymous namespace

std::shared_ptr<Scene> SceneBuilder::buildScene(const SceneBuilder::CreateInfo &createInfo) {
    switch (createInfo.format) {
    case SceneFormat::kMitsuba :
        return loadMitsubaScene(createInfo);
    }

    return nullptr;
}

} // namespace cobalt::render
