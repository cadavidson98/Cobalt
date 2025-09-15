#ifndef CBLT_IO_PNG_READER_H
#define CBLT_IO_PNG_READER_H

#include "math/math_types.h"

#include <memory>
#include <optional>
#include <string_view>

namespace cblt::io::png {

struct ByteImage {
    std::shared_ptr<float[]> data;
    uint32_t channelCount;
    vec2u size;
};

std::optional<ByteImage> read(const std::string_view fileName);

} // namespace cblt::io::png

#endif // CBLT_IO_PNG_READER_H
