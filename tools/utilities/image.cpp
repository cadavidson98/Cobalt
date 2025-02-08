#include "image.h"

#include "render/data/render_target.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {

// TODO: Taken from OpenEXR in an attempt to match blender output. Looks like I just need to gamma correct
// when using stbi_image
float knee(float base, float clamp) {
    return std::log(base * clamp + 1.f) / clamp;
}

uint8_t gamma(float linear, float grayPointCoefficient) {
    //
    // Conversion from half to unsigned char pixel data,
    // with gamma correction.  The conversion is the same
    // as in the exrdisplay program's ImageView class,
    // except with defog, kneeLow, and kneeHigh fixed
    // at 0.0, 0.0, and 5.0 respectively.
    //

    float grayPointValue = std::max(0.f, linear * grayPointCoefficient);

    grayPointValue = (grayPointValue > 1.f) ? 1.f + knee(grayPointValue - 1, 0.184874f) : grayPointValue;

    return uint8_t(std::clamp(std::pow(grayPointValue, 0.4545f) * 84.66f, 0.f, 255.f));
}

} // anonymous namespace

namespace cblt::cli {

Image::Image(const CreateInfo &createInfo): _renderTarget{createInfo.renderTarget} {
}

// TODO: need a png implementation that doesn't require gamma correction
bool Image::write(const WriteInfo &writeInfo) {
    if (!checkWriteInfo(writeInfo)) {
        return false;
    };

    static constexpr float kExposure = 1.f;

    const float linearToGrayPoint = std::pow(2.f, std::clamp(kExposure + 2.47393f, -20.f, 20.f));

    const auto floatToUchar = [linearToGrayPoint](const vec4f &vec) {
        static constexpr float kFloatToUChar = 255.f;
        return vec4u8{
            gamma(vec.x, linearToGrayPoint),
            gamma(vec.y, linearToGrayPoint),
            gamma(vec.z, linearToGrayPoint),
            uint8_t(std::clamp(kFloatToUChar * vec.w, 0.f, 255.f)),
        };
    };

    if (writeInfo.extension == "png") {
        const vec2u imageSize = _renderTarget->size();
        std::vector<vec4u8> imageBytes(imageSize.x * imageSize.y);
        for (uint32_t y = 0; y < imageSize.y; ++y) {
            for (uint32_t x = 0; x < imageSize.x; ++x) {
                const size_t index = y * imageSize.x + x;
                const vec4f &color = _renderTarget->at({x, y});
                imageBytes[index] = floatToUchar(color);
            }
        }

        return stbi_write_png(
            writeInfo.filePath.c_str(),
            imageSize.x,
            imageSize.y,
            4,
            imageBytes.data(),
            4 * imageSize.x
        );
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
