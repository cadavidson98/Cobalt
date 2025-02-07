#ifndef CBLT_RENDER_TEXTURE_H
#define CBLT_RENDER_TEXTURE_H

#include "color.h"
#include "size_types.h"
#include "vec2.h"

#include <array>
#include <memory>
#include <string>

namespace cblt::render {

class CoTexture {
public:
    struct CreateFromFileInfo {
        std::string fileName;
        std::string fileExtension;
    };

    static std::shared_ptr<CoTexture> create(const CreateFromFileInfo &createInfo);

    ~CoTexture();

    vec2f size() const;
    CoColor sample(const vec2f &uvCoord);

private:
    static constexpr std::array<const char *, 1> kValidFileTypes = {
        "exr",
    };

    enum PixelFormat {
        kPixelFormatRGBA16Float,
        kPixelFormatRGBA8UInt,
    };

    struct CreateFromBytesInfo {
        uint8_t *bytes;
        PixelFormat format;
        vec2f dimensions;
    };

    CoTexture() = delete;
    CoTexture(const CreateFromBytesInfo &createInfo);

    uint8_t *_textureData;
    PixelFormat _textureFormat;
    vec2f _textureSize;

    static bool _checkCreateInfo(const CreateFromFileInfo &createInfo);

    static std::shared_ptr<CoTexture> _loadFromEXR(const CreateFromFileInfo &createInfo);
};

} // namespace
  // cblt::render

#endif // CBLT_RENDER_TEXTURE_H
