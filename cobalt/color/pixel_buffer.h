#ifndef COBALT_COLOR_PIXEL_BUFFER_H
#define COBALT_COLOR_PIXEL_BUFFER_H

#include "rgb.h"

#include "core/size_types.h"
#include "math/math_types.h"

#include <memory>
#include <span>

namespace cobalt::color {

class PixelBuffer {
public:
    struct CreateInfo {
        rgb::Colorspace colorspace;
        vec2u size;
    };

    static std::shared_ptr<PixelBuffer> create(const CreateInfo &createInfo);

    rgb::Colorspace colorspace() const;
    vec2u size() const;

    rgb::Value &at(vec2u idx);

    std::span<rgb::Value> scanline(size_t row);
    std::span<const rgb::Value> scanline(size_t row) const;

private:
    PixelBuffer() = delete;
    PixelBuffer(const PixelBuffer &) = delete;
    PixelBuffer &operator=(PixelBuffer &) = delete;

    PixelBuffer(rgb::Colorspace colorspace, vec2u size);

    rgb::Colorspace _colorspace;
    vec2u _size;

    std::unique_ptr<rgb::Value[]> _data;
};

} // namespace cobalt::color

#endif // COBALT_RENDER_PIXEL_BUFFER_H
