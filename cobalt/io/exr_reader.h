#ifndef CBLT_IO_IMAGE_READER_H
#define CBLT_IO_IMAGE_READER_H

#include "core/size_types.h"
#include "math/math_types.h"

#include <OpenEXR/ImfPixelType.h>

#include <filesystem>
#include <optional>

namespace cblt::io::exr {

struct Image {
    std::shared_ptr<void> data;
    Imf::PixelType format;
    vec2u size;
    uint32_t channelCount;
};

std::optional<Image> read(const std::filesystem::path);

} // namespace cblt::io::exr

#endif // CBLT_IO_IMAGE_READER_H
