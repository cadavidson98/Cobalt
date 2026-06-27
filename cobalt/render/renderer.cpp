#include "renderer.h"

#include "component_storage.h"

#include "color/blackbody_spectrum.h"
#include "color/pixel_buffer.h"
#include "color/polynomial_spectrum.h"
#include "color/rgb.h"
#include "color/spectrum.h"
#include "color/xyz.h"
#include "core/logging.h"
#include "core/size_types.h"
#include "data/camera.h"
#include "geometry/bounding_volume_scene_storage.h"
#include "geometry/bounding_volume_types.h"
#include "geometry/ray.h"
#include "math/math_utilities.h"
#include "scene/scene.h"

#include <cstdint>
#include <memory>
#include <random>

namespace cobalt::render {

namespace {

class UniformSampler {
public:
    UniformSampler(): _device(), _generator(_device()), _distribution(0.f, 1.f) {
    }

    float operator()() {
        return _distribution(_generator);
    }

private:
    std::random_device _device;
    std::mt19937 _generator;
    std::uniform_real_distribution<> _distribution;
};

constexpr uint32_t kTileSize = 32;

template<size_t tileSize>
struct TileDispatchSize {
    static constexpr size_t kTileSize = tileSize;
    vec2u tilesPerGrid;
};

template<class TileKernel>
concept isTileKernel = requires(vec2u threadID, TileKernel kernel) {
    { kernel(threadID) } -> std::same_as<void>;
};

template<typename TileKernel, size_t tileSize>
    requires isTileKernel<TileKernel>
void dispatchTileWorkload(const TileDispatchSize<tileSize> &dispatchSize, TileKernel kernel) {
    // TODO: this should schedule tiles __in parallel__ using thread, openmp, etc.
    // dispatch "tiles"
    for (uint32_t tileY = 0; tileY < dispatchSize.tilesPerGrid.y; ++tileY) {
        for (uint32_t tileX = 0; tileX < dispatchSize.tilesPerGrid.x; ++tileX) {
            const vec2u threadStart = {
                uint32_t(dispatchSize.kTileSize * tileX),
                uint32_t(dispatchSize.kTileSize * tileY),
            };

            // dispatch "threads"
            for (uint32_t threadY = 0; threadY < dispatchSize.kTileSize; ++threadY) {
                for (uint32_t threadX = 0; threadX < dispatchSize.kTileSize; ++threadX) {
                    kernel(threadStart + vec2u{threadX, threadY});
                }
            }
        }
    }
}

struct CollisionKernel {
    vec2u size;
    std::shared_ptr<Camera> camera;
    geom::BoundingVolume<geom::SceneStorage> accelerator;
    std::shared_ptr<UniformSampler> sampler;

    std::shared_ptr<Camera::Sample[]> samples;
    std::shared_ptr<geom::IntersectionResult[]> results;

    void operator()(vec2u threadID) {
        if (threadID.x >= size.x || threadID.y >= size.y) {
            return;
        }

        const vec2f pixelPos = {
            .x = float(threadID.x),
            .y = float(threadID.y),
        };

        const vec2f viewSize = {
            .x = float(size.x),
            .y = float(size.y),
        };

        auto viewportToNDC = [viewSize](vec2f pixelPos) -> vec2f {
            return ((pixelPos / viewSize) * vec2f{.x = 2.f, .y = -2.f} + vec2f{.x = -1.f, .y = 1.f});
        };

        const size_t threadIdx = threadID.y * size.y + threadID.x;

        const vec2f ndcPos = viewportToNDC(pixelPos);

        const float sampleWeight = (*sampler)();

        const geom::Ray ray = camera->createRay(ndcPos);
        const Camera::Sample sample = camera->sampleWavelengths(sampleWeight);

        samples[threadIdx] = sample;
        results[threadIdx] = accelerator.intersects(ray);
    }
};

struct ResolveKernel {
    vec2u size;
    size_t samplesPerPixel;
    mat3f xyzToRGB;

    std::shared_ptr<Camera> camera;
    std::shared_ptr<Texture> environmentMap;

    std::shared_ptr<const color::BlackBodySpectrum[]> blackbodies;
    std::shared_ptr<const color::PolynomialSpectrum[]> polynomials;

    // std::shared_ptr<ComponentStorage> components;

    std::shared_ptr<const Camera::Sample[]> samples;
    std::shared_ptr<const geom::IntersectionResult[]> results;
    std::shared_ptr<color::PixelBuffer> pixelBuffer;

    void operator()(vec2u threadID) {
        if (threadID.x >= size.x || threadID.y >= size.y) {
            return;
        }

        const size_t threadIdx = threadID.y * size.y + threadID.x;

        const geom::IntersectionResult &result = results[threadIdx];

        const float weight = 1.f / float(samplesPerPixel);

        const vec4f wavelengths = samples[threadIdx].wavelengths;
        const vec4f pdfs = samples[threadIdx].pdfs;

        vec4f samples;
        // todo: this eventually won't be 'trivial' to evaluate here, because we will have rendering equations!
        if (result) {

            // Typed Index resolve here
            switch(result.spectrum.type) {
            case color::SpectrumType::kBlackbody:
                samples = blackbodies[result.spectrum.index][wavelengths];
                break;
            case color::SpectrumType::kPolynomial:
                samples = polynomials[result.spectrum.index][wavelengths];
                break;
            case color::SpectrumType::kSampled:
                samples = sampled[result.spectrum.index][wavelengths];
                break;
            }

        } else if (environmentMap) {
            // TODO: should env be implicit by nature of the bvh collision test and scene structure?
            const vec2f viewSize = {
                .x = float(size.x),
                .y = float(size.y),
            };

            // TODO: This seems like a lot of work...
            auto viewportToNDC = [viewSize](vec2f pixelPos) -> vec2f {
                return ((pixelPos / viewSize) * vec2f{.x = 2.f, .y = -2.f} + vec2f{.x = -1.f, .y = 1.f});
            };

            const geom::Ray ray = camera->createRay(viewportToNDC({
                .x = float(threadID.x),
                .y = float(threadID.y),
            }));

            float theta = std::atan2(-ray.dir[2], ray.dir[0]);
            theta = (theta < 0.f) ? theta + kPI : theta;
            const float phi = std::acos(ray.dir[1]);
            const float u = ((theta) / (2.f * kPI));
            const float v = phi / kPI;

            const vec2f coordinates = {
                .x = u,
                .y = v,
            };

            const color::PolynomialSpectrum spectrum = environmentMap->sample(coordinates);
            samples = spectrum[wavelengths];
        } else {
            return;
        }

        const color::xyz::Tristimulus xyz = color::xyz::convert(samples, wavelengths, pdfs);
        const vec3f xyzValue = {
            .x = xyz.x,
            .y = xyz.y,
            .z = xyz.z,
        };

        const vec3f rgbValue = xyzToRGB * xyzValue;

        const color::rgb::Value rgb = {
            .r = rgbValue.x,
            .g = rgbValue.y,
            .b = rgbValue.z,
        };

        color::rgb::Value &pixel = pixelBuffer->at(threadID);
        pixel += weight * rgb;
    }
};

} // anonymous namespace

bool render(std::shared_ptr<const Scene> scene, std::shared_ptr<color::PixelBuffer> pixelBuffer) {
    std::shared_ptr<render::Camera> camera = scene->camera();
    std::shared_ptr<geom::SceneStorage> sceneStorage = scene->storage();
    std::shared_ptr<ComponentStorage> componentStorage = scene->componentStorage();

    if (!camera) {
        CoLogError("Missing Camera");
        return false;
    }

    if (!sceneStorage || !componentStorage) {
        CoLogError("Missing Scene Storage");
        return false;
    }

    const vec2u viewSize = pixelBuffer->size();

    const TileDispatchSize<kTileSize> dispatchSize = {
        .tilesPerGrid = {
                         .x = utils::divUp(viewSize.x, uint32_t(kTileSize)),
                         .y = utils::divUp(viewSize.y, uint32_t(kTileSize)),
                         },
    };

    const size_t pixelCount = viewSize.x * viewSize.y;
    std::shared_ptr<geom::IntersectionResult[]> results = std::make_shared<geom::IntersectionResult[]>(pixelCount);
    std::shared_ptr<Camera::Sample[]> samples = std::make_shared<Camera::Sample[]>(pixelCount);

    auto mortonKeyer = [](const geom::MortonPrimitive &lhs) -> uint32_t {
        return lhs.mortonCode;
    };

    std::vector<geom::MortonPrimitive> mortonEncodedPrimitives = sceneStorage->mortonEncodePrimitives();

    core::radix_sort<30>(mortonEncodedPrimitives.begin(), mortonEncodedPrimitives.end(), mortonKeyer);

    // TODO: structure of array here; looks like I can "coarsen these reorders"
    sceneStorage->reorder(mortonEncodedPrimitives);
    componentStorage->reorder(mortonEncodedPrimitives);

    static constexpr size_t kSamplesPerPixel = 128;

    const mat3f xyzToRGB = color::rgb::convertFromXYZ(pixelBuffer->colorspace());

    CollisionKernel collisionKernel = {
        .size = viewSize,
        .camera = camera,
        .accelerator = geom::BoundingVolume<geom::SceneStorage>(sceneStorage, mortonEncodedPrimitives),
        .sampler = std::make_shared<UniformSampler>(),
        .samples = samples,
        .results = results,
    };

    ResolveKernel resolveKernel{
        .size = viewSize,
        .samplesPerPixel = kSamplesPerPixel,
        .xyzToRGB = xyzToRGB,
        .camera = camera,
        .environmentMap = scene->environmentMap(),
        .components = componentStorage,
        .samples = samples,
        .results = results,
        .pixelBuffer = pixelBuffer,
    };

    for (size_t idx = 0; idx < kSamplesPerPixel; ++idx) {
        dispatchTileWorkload(dispatchSize, collisionKernel);
        dispatchTileWorkload(dispatchSize, resolveKernel);
    }

    return true;
}

} // namespace cobalt::render
