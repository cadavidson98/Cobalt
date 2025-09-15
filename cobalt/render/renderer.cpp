#include "renderer.h"

#include "core/logging.h"
#include "core/size_types.h"
#include "data/camera.h"
#include "data/color.h"
#include "data/render_target.h"
#include "geometry/bounding_volume_scene_storage.h"
#include "geometry/bounding_volume_types.h"
#include "geometry/ray.h"
#include "math/math_utilities.h"
#include "scene/scene.h"

namespace cblt::render {

namespace {

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
    std::shared_ptr<CoCamera> camera;
    geom::CoBoundingVolume<geom::CoSceneStorage> accelerator;

    std::shared_ptr<geom::IntersectionResult[]> results = {};

    void operator()(vec2u threadID) {
        if (threadID.x >= size.x || threadID.y >= size.y) {
            return;
        }

        const vec2f pixelPos = {
            float(threadID.x),
            float(threadID.y),
        };

        const vec2f viewSize = {
            float(size.x),
            float(size.y),
        };

        auto viewportToNDC = [viewSize](vec2f pixelPos) -> vec2<float> {
            return ((pixelPos / viewSize) * vec2f{2.f, -2.f} + vec2f{-1.f, 1.f});
        };

        const size_t threadIdx = threadID.y * size.y + threadID.x;

        const vec2f ndcPos = viewportToNDC(pixelPos);

        const geom::CoRay ray = camera->createRay(ndcPos);
        results[threadIdx] = accelerator.intersects(ray);
    }
};

struct ResolveKernel {
    vec2u size;
    std::shared_ptr<CoRenderTarget> renderTarget = {};
    std::shared_ptr<const geom::IntersectionResult[]> results = {};

    void operator()(vec2u threadID) {
        if (threadID.x >= size.x || threadID.y >= size.y) {
            return;
        }

        const size_t threadIdx = threadID.y * size.y + threadID.x;

        const geom::IntersectionResult &result = results[threadIdx];
        if (result) {
            std::array<render::CoColor, 4> colors{
                {
                 {1.f, 0.f, 0.f, 1.f},
                 {0.f, 0.f, 1.f, 1.f},
                 {0.5f, 0.5f, 0.5f, 1.f},
                 {0.f, .6f, .9f, 1.f},
                 }
            };
            renderTarget->write(threadID, colors[result.geometry.index % 4]);
        }
    }
};

} // anonymous namespace

bool render(const CoScene &scene, std::shared_ptr<CoRenderTarget> renderTarget) {
    std::shared_ptr<render::CoCamera> camera = scene.camera();
    std::shared_ptr<geom::CoSceneStorage> sceneStorage = scene.storage();

    if (!camera) {
        CoLogError("Missing Camera");
        return false;
    }

    if (!sceneStorage) {
        CoLogError("Missing Scene Storage");
        return false;
    }

    const vec2u viewSize = renderTarget->size();

    const TileDispatchSize<kTileSize> dispatchSize{
        .tilesPerGrid =
            {
                           utils::divUp(viewSize.x, uint32_t(kTileSize)),
                           utils::divUp(viewSize.y, uint32_t(kTileSize)),
                           }
    };

    const size_t pixelCount = viewSize.x * viewSize.y;
    std::shared_ptr<geom::IntersectionResult[]> results = std::make_shared<geom::IntersectionResult[]>(pixelCount);

    CollisionKernel collisionKernel{
        .size = viewSize,
        .camera = camera,
        .accelerator = geom::CoBoundingVolume<geom::CoSceneStorage>({sceneStorage}),
        .results = results,
    };

    dispatchTileWorkload(dispatchSize, collisionKernel);

    ResolveKernel resolveKernel{
        .size = viewSize,
        .renderTarget = renderTarget,
        .results = results,
    };

    dispatchTileWorkload(dispatchSize, resolveKernel);

    return true;
}
} // namespace cblt::render
