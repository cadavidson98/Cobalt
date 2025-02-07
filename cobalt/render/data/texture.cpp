#include "texture.h"

#include "OpenEXR/ImfChannelList.h"
#include "OpenEXR/ImfRgbaFile.h"
#include "logging.h"
#include "texture_utilities.h"
#include "vec4.h"

#include <algorithm>

namespace cblt::render {

std::shared_ptr<CoTexture> CoTexture::create(const CreateFromFileInfo &createInfo) {
    if (!_checkCreateInfo(createInfo)) {
        return nullptr;
    }

    if (createInfo.fileExtension == "exr") {
        return _loadFromEXR(createInfo);
    }

    CoLogError("Unsupported texture format");
    return nullptr;
}

vec2f CoTexture::size() const {
    return _textureSize;
}

CoColor CoTexture::sample(const vec2f &uvCoord) {
    // rescale to image space
    const vec2f texel = uvCoord * _textureSize;
    if (std::clamp(texel.x, 0.f, _textureSize.x - 1.f) != texel.x ||
        std::clamp(texel.y, 0.f, _textureSize.y - 1.f) != texel.y) {
        return CoColor{0.f, 0.f, 0.f, 0.f};
    }

    vec4f textureColor = vec4f{0.f, 0.f, 0.f, 0.f};
    switch (_textureFormat) {
    case kPixelFormatRGBA16Float : {
        const Imf::Rgba *textureDataFloat16 = reinterpret_cast<Imf::Rgba *>(_textureData);
        const vec2i nearestTexel = {int(std::round(texel.x)), int(std::round(texel.y))};
        static constexpr int kNeighborhood = 3;
        static constexpr int kHalfNeighborhood = kNeighborhood >> 1;
        const vec2i start = vec2i(nearestTexel.x - kHalfNeighborhood, nearestTexel.y - kHalfNeighborhood);
        const vec2i end = vec2i(nearestTexel.x + kHalfNeighborhood + 1, nearestTexel.y + kHalfNeighborhood + 1);
        for (int yTexel = std::max(0, start.y); yTexel < std::min(int(_textureSize.y), end.y); ++yTexel) {
            for (int xTexel = std::max(0, start.x); xTexel < std::min(int(_textureSize.x), end.x); ++xTexel) {
                const vec2f neighborTexel{float(xTexel), float(yTexel)};
                const Imf::Rgba &color = textureDataFloat16[yTexel * uint32_t(_textureSize.x) + xTexel];

                const vec4f neighborColor{
                    float(color.r),
                    float(color.g),
                    float(color.b),
                    float(color.a),
                };

                const vec4f scaledColor = tentFilter(neighborTexel, texel, neighborColor);
                textureColor = textureColor + scaledColor;
            }
        }

        break;
    }
    }

    const vec4f clampedColor = clamp(textureColor, 0.f, 1.f);
    return CoColor{textureColor.x, textureColor.y, textureColor.z, textureColor.w};
}

CoTexture::CoTexture(const CreateFromBytesInfo &createInfo) {
    _textureData = createInfo.bytes;
    _textureFormat = createInfo.format;
    _textureSize = createInfo.dimensions;
}

CoTexture::~CoTexture() {
    delete[] static_cast<uint8_t *>(_textureData);
}

bool CoTexture::_checkCreateInfo(const CreateFromFileInfo &createInfo) {
    if (std::find(kValidFileTypes.begin(), kValidFileTypes.end(), createInfo.fileExtension) == kValidFileTypes.end()) {
        CoLogError("Invalid file type");
        return false;
    }

    return true;
}

std::shared_ptr<CoTexture> CoTexture::_loadFromEXR(const CreateFromFileInfo &createInfo) {
    // TODO: make array of half4 as it will support loading EXR trivially and API agnostic
    Imf::Rgba *pixelBuffer = nullptr;
    try {
        Imf::RgbaInputFile inputFile(createInfo.fileName.c_str(), 1);

        const Imath::Box2i window = inputFile.dataWindow();
        const Imath::V2i windowSize(window.max.x - window.min.x + 1, window.max.y - window.min.y + 1);
        pixelBuffer = new Imf::Rgba[windowSize.x * windowSize.y];
        const int dx = window.min.x;
        const int dy = window.min.y;
        inputFile.setFrameBuffer(pixelBuffer, 1, windowSize.x);
        inputFile.readPixels(window.min.y, window.max.y);
        inputFile.parts();
        return std::shared_ptr<CoTexture>(new CoTexture(
            CreateFromBytesInfo{
                .bytes = reinterpret_cast<uint8_t *>(pixelBuffer),
                .format = kPixelFormatRGBA16Float,
                .dimensions = {
                               float(windowSize.x),
                               float(windowSize.y),
                               },
        }
        ));
    } catch (Iex::BaseExc &e) {
        CoLogError(e.what());
        return nullptr;
    }
}

} // namespace cblt::render
