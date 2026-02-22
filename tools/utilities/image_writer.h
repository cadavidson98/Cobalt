#ifndef COBALT_CLI_IMAGE_WRITER_H
#define COBALT_CLI_IMAGE_WRITER_H

#include "color/pixel_buffer.h"

#include <string>

namespace cblt {

namespace color {
class PixelBuffer;
}

namespace cli {

enum class ImageType {
    kPNG,
    kEXR,
};

struct WriteInfo {
    std::string fileName;
    ImageType type;
    std::shared_ptr<color::PixelBuffer> pixelBuffer;
};

bool writeImage(const WriteInfo &writeInfo);

} // namespace cli

} // namespace cblt

#endif // COBALT_CLI_IMAGE_WRITER_H
