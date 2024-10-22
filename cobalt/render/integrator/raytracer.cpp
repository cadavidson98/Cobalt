#include "raytracer.h"

#include "camera.h"
#include "math_utils.h"
#include "ray.h"

#include <atomic>

namespace cblt::render {

std::shared_ptr<Raytracer> Raytracer::create(const Raytracer::CreateInfo &createInfo) {
    if (_checkCreateInfo(createInfo)) {
        return nullptr;
    }

    return std::shared_ptr<Raytracer>(new Raytracer());
}

Raytracer::~Raytracer() {
}

std::optional<Raytracer::RenderTargets> Raytracer::render(const Raytracer::DrawInfo &drawInfo) {
    if (!_checkDrawInfo(drawInfo)) {
        return std::nullopt;
    }

    // todo: configure render targets?

    static const vec2u tileSize = {kTileSize, kTileSize};
    const vec2u gridSize = divUp(_renderTargetSize, tileSize);
    const float numTiles = float(gridSize.x * gridSize.y);
    std::atomic_uint32_t numTilesRendered = 0;
    for (uint32_t tileY = 0; tileY < gridSize.y; ++tileY) {
        for (uint32_t tileX = 0; tileX < gridSize.x; ++tileX) {
            _renderTile(drawInfo, {tileX, tileY});
            numTilesRendered.fetch_add(1, std::memory_order_relaxed);
            if (drawInfo.progressCallback) {
                drawInfo.progressCallback.value()(float(numTilesRendered) / float(numTiles));
            }
        }
    }
}

void Raytracer::_renderTile(const DrawInfo &drawInfo, vec2u tileIdx) {
    // todo: compute pixel
    const vec2u pixelStart = tileIdx * kTileSize;
    const vec2u pixelEnd = {
        std::min(pixelStart.x + kTileSize, _renderTargetSize.x),
        std::min(pixelStart.y + kTileSize, _renderTargetSize.y),
    };

    for (uint32_t pixelY = pixelStart.y; pixelY < pixelEnd.y; ++pixelY) {
        for (uint32_t pixelX = pixelStart.x; pixelX < pixelEnd.x; ++pixelX) {
            const geom::CoRay &ray = drawInfo.camera.get().createRay({pixelX, pixelY});
            for (uint32_t rayDepth = 0; rayDepth < drawInfo.drawOptions.maxDepth; ++rayDepth) {
                // intersect with scene
                // drawInfo.scene.get().intersectClosest(ray);
                // render
                // recurse
                // ray =
            }
        }
    }
}

bool Raytracer::_checkCreateInfo(const CreateInfo &createInfo) {
    return true;
}

bool Raytracer::_checkDrawInfo(const DrawInfo &drawInfo) {
    return true;
}

} // namespace cblt::render
