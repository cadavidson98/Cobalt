#include "image.h"

#include "render_target.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace cblt::cli {

Image::Image(const CreateInfo &createInfo): _renderTarget{createInfo.renderTarget} {
}

bool Image::write(const WriteInfo &writeInfo) {
    if (!checkWriteInfo(writeInfo)) {
        return false;
    };

    const auto floatToUchar = [](const vec4f &vec) {
        static constexpr float kFloatToUChar = 255.f;
        return vec4u8(
            uint8_t(kFloatToUChar * vec.x),
            uint8_t(kFloatToUChar * vec.y),
            uint8_t(kFloatToUChar * vec.z),
            uint8_t(kFloatToUChar * vec.w)
        );
    };

    if (writeInfo.extension == "png") {
        const vec2u imageSize = _renderTarget->size();
        std::vector<vec4u8> imageBytes(imageSize.x * imageSize.y);
        for (uint32_t x = 0; x < imageSize.x; ++x) {
            for (uint32_t y = 0; y < imageSize.y; ++y) {
                size_t index = y * imageSize.x + x;
                const vec4f &color = _renderTarget->at({x, y});
                imageBytes[index] = floatToUchar(color);
            }
        }
        return stbi_write_png(writeInfo.filePath.c_str(), imageSize.x, imageSize.y, 4, imageBytes.data(), 0);
    } else if (writeInfo.extension == "hdr") {
        const vec2u imageSize = _renderTarget->size();
        return stbi_write_hdr(
            writeInfo.filePath.c_str(),
            imageSize.x,
            imageSize.y,
            4,
            reinterpret_cast<float *>(_renderTarget->data())
        );
    }

    return true;
}

bool Image::checkCreateInfo(const CreateInfo &createInfo) {
    if (!createInfo.renderTarget) {
        return false;
    }

    if (createInfo.renderTarget->tiling() != render::CoRenderTarget::RenderTargetTilingLinear) {
        return false;
    }

    return true;
}

bool Image::checkWriteInfo(const WriteInfo &writeInfo) {
    const auto isValidExtension = [&writeInfo](const char *extension) {
        return std::strcmp(writeInfo.extension.c_str(), extension) == 0;
    };

    if (std::find_if(kValidFileExtensions.begin(), kValidFileExtensions.end(), isValidExtension) ==
        kValidFileExtensions.end()) {
        return false;
    }

    return true;
}

std::unique_ptr<Image> Image::create(const CreateInfo &createInfo) {
    if (!checkCreateInfo(createInfo)) {
        return nullptr;
    }

    return std::unique_ptr<Image>(new Image(createInfo));
}

} // namespace cblt::cli
