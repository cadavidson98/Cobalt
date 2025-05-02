#ifndef COBALT_RENDER_RENDERTARGET_H
#define COBALT_RENDER_RENDERTARGET_H

#include "core/size_types.h"
#include "math/math_types.h"
#include "render/data/color.h"

#include <memory>
#include <span>

namespace cblt::render {

class CoRenderTarget {
public:
    struct CreateInfo {
        vec2u size = {0, 0};
    };

    static std::shared_ptr<CoRenderTarget> create(const CreateInfo &createInfo);
    ~CoRenderTarget();

    void write(const vec2u &idx, const CoColor &color);
    const CoColor &at(const vec2u idx) const;

    vec2u size() const;
    std::span<const CoColor> data() const;

private:
    CoRenderTarget() = delete;
    CoRenderTarget(const vec2u &size);

    CoColor *_renderTargetBytes;
    vec2u _renderTargetSize;
};
} // namespace cblt::render

#endif // COBALT_RENDER_RENDERTARGET_H
