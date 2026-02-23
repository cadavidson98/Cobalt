#include "pixel_buffer.h"

#include "color/rgb.h"
#include "core/logging.h"

#include <memory>

namespace cobalt::color {

namespace {

[[nodiscard]] bool checkCreateInfo(const PixelBuffer::CreateInfo &createInfo) {
    if (createInfo.size.x == 0 || createInfo.size.y == 0) {
        CoLogError("Invalid pixel buffer size");
        return false;
    }

    return true;
}

} // anonymous namespace

PixelBuffer::PixelBuffer(rgb::Colorspace colorspace, vec2u size): _colorspace{colorspace}, _size{size} {
    _data = std::make_unique<rgb::Value[]>(size.x * size.y);
}

std::shared_ptr<PixelBuffer> PixelBuffer::create(const CreateInfo &createInfo) {
    if (!checkCreateInfo(createInfo)) {
        return nullptr;
    }

    std::shared_ptr<PixelBuffer> pixelBuffer =
        std::shared_ptr<PixelBuffer>(new PixelBuffer(createInfo.colorspace, createInfo.size));

    return pixelBuffer;
}

vec2u PixelBuffer::size() const {
    return _size;
}

rgb::Colorspace PixelBuffer::colorspace() const {
    return _colorspace;
}

rgb::Value &PixelBuffer::at(vec2u idx) {
    const size_t f = idx.x + idx.y * _size.x;
    return _data[f];
}

std::span<const rgb::Value> PixelBuffer::scanline(size_t row) const {
    return std::span<const rgb::Value>(_data.get() + (row * _size.x), _size.x);
}

} // namespace cobalt::color
