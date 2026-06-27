#include "scene_builder.h"

#include "component_storage.h"
#include "scene.h"
#include "texture.h"

#include "color/blackbody_spectrum.h"
#include "color/pixel_buffer.h"
#include "color/polynomial_spectrum.h"
#include "color/rgb.h"
#include "color/spectrum.h"
#include "color/xyz.h"
#include "core/logging.h"
#include "geometry/bounding_volume_scene_storage.h"
#include "geometry/bounding_volume_types.h"
#include "geometry/mesh.h"
#include "geometry/sphere.h"
#include "io/image/exr.h"
#include "io/image/image.h"
#include "io/image/png.h"
#include "io/mitsuba.h"
#include "io/obj_reader.h"
#include "math/math_types.h"
#include "rgb2spec/rgb2spec.h"

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

        _rgbToSpec = rgb2spec_load(RGB2SPEC_COLOR_SRGB);
    }

    virtual ~SceneDelegate() {
        rgb2spec_free(_rgbToSpec);
    };

    bool readSphere(const io::mitsuba::Shape<io::mitsuba::Sphere> &sphere) override {
        const std::optional<TypedIndex<color::SpectrumType>> spectrumIndex = readSpectrum(sphere.spectrum);
        if (!spectrumIndex) {
            return false;
        }

        geom::Sphere sphereGeometry = {
            .center = simd::vec3f(sphere.shape.center.x, sphere.shape.center.y, sphere.shape.center.z),
            .radius = sphere.shape.radius,
        };

        _spheres.push_back(sphereGeometry);

        const ComponentStorage::Primitive spherePrimitive = {
            .components = ComponentStorage::Tag::kGeometry | ComponentStorage::Tag::kSpectrum,            
            .geometryIdx = TypedIndex<geom::Shape> {
                .type = geom::Shape::kSphere,
                .idx = uint32_t(_spheres.size() - 1),
            },
            .spectrumIdx = *spectrumIndex,
        };

        _primitives.push_back(spherePrimitive);

        return true;
    }

    bool readMesh(const io::mitsuba::Shape<io::mitsuba::Mesh> &mesh) override {
        const std::optional<TypedIndex<color::SpectrumType>> spectrumIndex = readSpectrum(mesh.spectrum);
        if (!spectrumIndex) {
            return false;
        }

        std::shared_ptr<geom::Mesh> meshGeometry;
        if (mesh.shape.fileExtension == "obj") {
            const std::filesystem::path filePath = _rootDirectory / mesh.shape.fileName;
            const std::optional<io::obj::Mesh> objMesh = io::obj::read(filePath.c_str());
            if (objMesh) {
                meshGeometry = geom::Mesh::create({
                    .positions = objMesh->positions,
                });
            }
        }

        if (!meshGeometry) {
            return false;
        }

        _meshes.push_back(meshGeometry);

        const ComponentStorage::Primitive meshPrimitive = {
            .components = ComponentStorage::Tag::kGeometry | ComponentStorage::Tag::kSpectrum,
            .geometryIdx = TypedIndex<geom::Shape> {
                .type = geom::Shape::kMesh,
                .idx = uint32_t(_meshes.size() - 1),
            },
            .spectrumIdx = *spectrumIndex,
        };

        _primitives.push_back(meshPrimitive);

        return true;
    }

    bool readEmitter(const io::mitsuba::Emitter &emitter) override {
        if (emitter.emissionMap) {
            const std::filesystem::path filePath = _rootDirectory / emitter.emissionMap.fileName;
            _environmentMap = readTexture(filePath);
        }

        const std::optional<TypedIndex<color::SpectrumType>> spectrumIndex = readSpectrum(emitter.radiance);
        if (!spectrumIndex) {
            return false;
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

    std::vector<geom::Sphere> _spheres = {};
    // simple idea: 'create' method takes an allocator, so you can have arrays of pointers be initialized on contiguous memory
    std::vector<std::shared_ptr<geom::Mesh>> _meshes = {};

    std::vector<color::BlackBodySpectrum> _blackbodies = {};
    std::vector<color::PolynomialSpectrum> _polynomials = {};

    std::vector<ComponentStorage::Primitive> _primitives = {};

    std::shared_ptr<geom::SceneStorage> _sceneGeometry;
    std::shared_ptr<ComponentStorage> _sceneComponents;

    std::shared_ptr<Camera> _camera = {};

    RGB2Spec *_rgbToSpec = {};

    // I am allowed to do whatever I want; not defined on Mitsuba schema as supporting 'id' parameter
    // Q: do I need to 'hash' or 'cache' a spectrum?
    // A: Yes! This is 'kind of' a material system, until we get PROPER bsdf and emitter support?
    std::optional<TypedIndex<color::SpectrumType>> readSpectrum(const io::mitsuba::Spectrum &spectrum) {
        switch (spectrum) {
            case io::mitsuba::Spectrum::Type::kRGB: {
                std::array<float, color::PolynomialSpectrum::kCoefficientsCount> coefficients;
                rgb2spec_fetch(
                    _rgbToSpec, 
                    const_cast<float *>(&spectrum.rgb.r), 
                    coefficients.data()
                );

                _polynomials.emplace_back(coefficients);

                return TypedIndex<color::SpectrumType> {
                    .type = color::SpectrumType::kPolynomial,
                    .idx = uint32_t(_polynomials.size() - 1),
                };
            }
            case io::mitsuba::Spectrum::Type::kBlackBody: {
                _blackbodies.emplace_back(spectrum.blackbody.temperature);
                return TypedIndex<color::SpectrumType> {
                    .type = color::SpectrumType::kBlackbody,
                    .idx = uint32_t(_blackbodies.size() - 1),
                };
            }
            case io::mitsuba::Spectrum::Type::kNone: {
                return std::nullopt;
            }
        }

        // should never hit, just needed by compiler
        return std::nullopt;
    }
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
