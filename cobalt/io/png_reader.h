#ifndef COBALT_IO_PNG_READER_H
#define COBALT_IO_PNG_READER_H

#include "math/math_types.h"

#include <filesystem>
#include <memory>
#include <optional>

namespace cobalt::io::png {

struct ByteImage {
    std::shared_ptr<float[]> data;
    uint32_t channelCount;
    vec2u size;
};

std::optional<ByteImage> read(const std::filesystem::path filePath);

} // namespace cobalt::io::png

#endif // COBALT_IO_PNG_READER_H
