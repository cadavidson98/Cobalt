#ifndef COBALT_CLI_IMAGE_WRITER_H
#define COBALT_CLI_IMAGE_WRITER_H

#include <array>
#include <memory>
#include <string>

namespace cblt {

namespace render {
class CoRenderTarget;
} // namespace render

namespace cli {

enum class ImageType {
    kPNG,
    kEXR,
};

struct WriteInfo {
    std::string fileName;
    ImageType type;
    std::reference_wrapper<render::CoRenderTarget> renderTarget;
};

bool writeImage(const WriteInfo &writeInfo);

} // namespace cli

} // namespace cblt

#endif // COBALT_CLI_IMAGE_WRITER_H
