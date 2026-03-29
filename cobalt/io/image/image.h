#ifndef COBALT_IO_IMAGE_H
#define COBALT_IO_IMAGE_H

#include "core/size_types.h"
#include "math/math_types.h"

#include <span>

namespace cobalt::io::image {

enum class Channel : uint8_t {
    kRed = 1 << 0,
    kBlue = 1 << 1,
    kGreen = 1 << 2,
    kAlpha = 1 << 3,
};

using ChannelFlags = uint8_t;

struct ColorSpace {
    vec2f whitePoint;
    vec2f red;
    vec2f green;
    vec2f blue;
};

struct Metadata {
    vec2u size;
    uint32_t channelCount;
    ChannelFlags channels;
    ColorSpace colorspace;
};

class FileReaderDelegate {
public:
    virtual ~FileReaderDelegate() = default;

    [[nodiscard]] virtual bool readMetadata(const Metadata &metadata) = 0;
    [[nodiscard]] virtual bool readRow(size_t row, std::span<const float> values) = 0;
};

class FileWriterDelegate {
public:
    virtual ~FileWriterDelegate() = default;

    [[nodiscard]] virtual Metadata metadata() const = 0;
    [[nodiscard]] virtual bool row(size_t rowIdx, std::span<float> values) = 0;
};

} // namespace cobalt::io::image

#endif // COBALT_IO_IMAGE_H
