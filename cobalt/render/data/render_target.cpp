#include "render_target.h"

namespace cblt::render {

namespace {

bool checkCreateInfo(const CoRenderTarget::CreateInfo &createInfo) {
    if (createInfo.size.x == 0 || createInfo.size.y == 0) {
        return false;
    }

    return true;
}

} // namespace

std::shared_ptr<CoRenderTarget> CoRenderTarget::create(const CoRenderTarget::CreateInfo &createInfo) {
    if (!checkCreateInfo(createInfo)) {
        return nullptr;
    }

    std::shared_ptr<CoRenderTarget> renderTarget = std::shared_ptr<CoRenderTarget>(new CoRenderTarget(createInfo.size));

    return renderTarget;
}

CoRenderTarget::~CoRenderTarget() {
    delete[] _renderTargetBytes;
}

void CoRenderTarget::write(const vec2u &renderTargetIdx, const CoColor &color) {
    size_t idx = renderTargetIdx.x + renderTargetIdx.y * _renderTargetSize.x;
    _renderTargetBytes[idx] = color;
}

CoRenderTarget::CoRenderTarget(const vec2u &size): _renderTargetSize{size} {

    const size_t numPixels = size_t(size.x) * size_t(size.y);
    CoColor *bytes = new CoColor[numPixels];

    _renderTargetBytes = bytes;
    _renderTargetSize = size;
}

vec2u CoRenderTarget::size() const {
    return _renderTargetSize;
}

std::span<const CoColor> CoRenderTarget::data() const {
    return std::span<const CoColor>(_renderTargetBytes, _renderTargetSize.x * _renderTargetSize.y);
}

const CoColor &CoRenderTarget::at(const vec2u idx) const {
    return _renderTargetBytes[idx.x + idx.y * _renderTargetSize.x];
}

} // namespace
  // cblt::render
