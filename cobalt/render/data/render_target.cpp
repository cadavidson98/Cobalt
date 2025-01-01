#include "render_target.h"

namespace cblt::render {

std::shared_ptr<CoRenderTarget> CoRenderTarget::create(const CoRenderTarget::CreateInfo &createInfo) {
    std::shared_ptr<CoRenderTarget> renderTarget = std::shared_ptr<CoRenderTarget>(new CoRenderTarget);
    if (!renderTarget->Init(createInfo)) {
        return nullptr;
    }

    return renderTarget;
}

CoRenderTarget::~CoRenderTarget() {
    delete[] renderTargetBytes;
}

void CoRenderTarget::Write(const vec2u &renderTargetIdx, const vec4f &value) {
    size_t idx = CoRenderTarget::_CalculateIndex(renderTargetIdx, renderTargetSize, renderTargetTiling);
    renderTargetBytes[idx] = value;
}

CoRenderTarget::CoRenderTarget() {
}

bool CoRenderTarget::Init(const CoRenderTarget::CreateInfo &createInfo) {
    if (!_CheckCreateInfo(createInfo)) {
        return false;
    }

    const size_t numPixels = size_t(createInfo.size.x) * size_t(createInfo.size.y);
    vec4f *bytes = new vec4f[numPixels];
    if (!numPixels) {
        return false;
    }

    renderTargetBytes = bytes;
    renderTargetSize = createInfo.size;
    renderTargetFormat = createInfo.format;
    renderTargetTiling = createInfo.tiling;

    return true;
}

bool CoRenderTarget::_CheckCreateInfo(const CoRenderTarget::CreateInfo &createInfo) {
    if (createInfo.size.x == 0 || createInfo.size.y == 0) {
        return false;
    }

    return true;
}

size_t CoRenderTarget::_CalculateIndex(const vec2u &idx, const vec2u &size, RenderTargetTiling tiling) {
    if (tiling == RenderTargetTilingLinear) {
        return size_t(idx.y) * size_t(size.x) + size_t(idx.x);
    } else if (tiling == RenderTargetTilingOptimal) {
        // find the tile
        const vec2u tileIdx = {idx.x / kTileSize, idx.y / kTileSize};
        const vec2u pixelOffset = {idx.x % kTileSize, idx.y % kTileSize};
        const size_t tilesPerRow = size.x / kTileSize;
        const size_t tileOffset = tilesPerRow * tileIdx.y + tileIdx.x;

        return tileOffset * kPixelsPerTile + pixelOffset.y * kTileSize + pixelOffset.x;
    }
    return 0;
}

vec2u CoRenderTarget::size() const {
    return renderTargetSize;
}

CoRenderTarget::RenderTargetFormat CoRenderTarget::format() const {
    return renderTargetFormat;
}

CoRenderTarget::RenderTargetTiling CoRenderTarget::tiling() const {
    return renderTargetTiling;
}

uint8_t *CoRenderTarget::data() const {
    return reinterpret_cast<uint8_t *>(renderTargetBytes);
}

const vec4f &CoRenderTarget::at(const vec2u idx) const {
    return renderTargetBytes[_CalculateIndex(idx, renderTargetSize, renderTargetTiling)];
}

} // namespace
  // cblt::render
