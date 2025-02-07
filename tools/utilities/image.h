#ifndef COBALT_CLI_IMAGE_H
#define COBALT_CLI_IMAGE_H

#include <array>
#include <memory>
#include <string>

namespace cblt {

namespace render {
class CoRenderTarget;
} // namespace render

namespace cli {

class Image {
public:
    struct CreateInfo {
        std::shared_ptr<render::CoRenderTarget> renderTarget;
    };

    struct WriteInfo {
        std::string filePath;
        std::string extension;
        // TODO: hdr, compression, etc.
    };

    bool write(const WriteInfo &writeInfo);

    static std::unique_ptr<Image> create(const CreateInfo &createInfo);

private:
    static constexpr std::array<const char *, 4> kValidFileExtensions = {"png", "jpg", "jpeg", "hdr"};

    std::shared_ptr<render::CoRenderTarget> _renderTarget;

    Image() = delete;
    Image(const CreateInfo &createInfo);

    static bool checkCreateInfo(const CreateInfo &createInfo);
    static bool checkWriteInfo(const WriteInfo &writeInfo);
};

} // namespace cli

} // namespace cblt

#endif // COBALT_CLI_IMAGE_H
