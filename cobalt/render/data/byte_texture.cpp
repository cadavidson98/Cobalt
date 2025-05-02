#include "byte_texture.h"

#include "color.h"
#include "texture_utilities.h"

#include "core/logging.h"
#include "math/math_types.h"

#include <Imath/half.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfRgba.h>

#include <algorithm>
#include <cassert>
#include <span>

namespace cblt::render {

namespace {

bool checkCreateInfo(const CoByteTexture::CreateFromBytesInfo &createInfo) {
    if (!createInfo.bytes) {
        CoLogError("'bytes' must be nonnull");
        return false;
    }

    if (createInfo.format == CoPixelFormat::Invalid) {
        CoLogError("pixel format must not be 'Invalid'");
        return false;
    }

    if (createInfo.dimensions.x == 0.f || createInfo.dimensions.y == 0) {
        CoLogError("'dimensions' must be greater than 0");
        return false;
    }

    return true;
}

template<typename T, uint32_t windowSize = 3>
CoColor
sampleNeighborhood(std::span<const T> data, const uint32_t numChannels, const vec2u textureSize, const vec2u center) {

    const uint32_t maxIdx = numChannels * textureSize.x * textureSize.y;

    const uint32_t halfWindow = windowSize >> 1;
    const vec2f centerFloat = {float(center.x), float(center.y)};

    vec4f sampleColor = {0.f, 0.f, 0.f, 0.f};

    const vec2i start = vec2i(center.x - halfWindow, center.y - halfWindow);
    const vec2i end = vec2i(center.x + halfWindow + 1, center.y + halfWindow + 1);

    for (int32_t yTexel = std::max(0, start.y); yTexel < std::min(int32_t(textureSize.y), end.y); ++yTexel) {
        for (int32_t xTexel = std::max(0, start.x); xTexel < std::min(int32_t(textureSize.x), end.x); ++xTexel) {
            const vec2f neighborTexel{float(xTexel), float(yTexel)};
            const uint32_t index = (yTexel * uint32_t(textureSize.x) + xTexel) * numChannels;

            assert(index < maxIdx);

            const float base(data[index]);

            const vec4f neighborColor{
                base,
                numChannels > 1 ? float(data[index + 1]) : 0.f,
                numChannels > 2 ? float(data[index + 2]) : 0.f,
                numChannels > 3 ? float(data[index + 3]) : 0.f,
            };

            const vec4f scaledColor = tentFilter(neighborTexel, centerFloat, neighborColor);
            sampleColor = sampleColor + scaledColor;
        }
    }

    return CoColor(sampleColor.x, sampleColor.y, sampleColor.z, sampleColor.w);
}

} // anonymous namespace

CoByteTexture::CoByteTexture(const CreateFromBytesInfo &createInfo) {
    _textureData = createInfo.bytes;
    _textureSize = createInfo.dimensions;
    _textureFormat = createInfo.format;

    _numChannels = createInfo.numChannels;
}

CoByteTexture::~CoByteTexture() {
}

CoPixelFormat CoByteTexture::format() const {
    return _textureFormat;
}

vec2u CoByteTexture::size() const {
    return _textureSize;
}

size_t CoByteTexture::size_bytes() const {
    const size_t bytesPerChannel = (_textureFormat == CoPixelFormat::Half) ? sizeof(Imath::half) : sizeof(float);
    return _textureSize.x * _textureSize.y * _numChannels * bytesPerChannel;
}

CoColor CoByteTexture::sample(const vec2f &uvCoord) const {
    // rescale to image space
    if (std::clamp(uvCoord.x, 0.f, 1.f) != uvCoord.x || std::clamp(uvCoord.y, 0.f, 1.f) != uvCoord.y) {
        return CoColor{0.f, 0.f, 0.f, 0.f};
    }

    const vec2f texel = uvCoord * vec2f{float(_textureSize.x), float(_textureSize.y)};
    const vec2u nearestTexel = {uint32_t(std::round(texel.x)), uint32_t(std::round(texel.y))};

    const size_t numTexels = _textureSize.x * _textureSize.y;

    CoColor textureColor = CoColor{0.f, 0.f, 0.f, 0.f};
    switch (_textureFormat) {
    case CoPixelFormat::Half : {
        std::span<const Imath::half> samples(static_cast<const Imath::half *>(_textureData.get()), numTexels);
        textureColor = sampleNeighborhood(samples, _numChannels, _textureSize, nearestTexel);
    }
    case CoPixelFormat::Float : {
        std::span<const float> samples(static_cast<const float *>(_textureData.get()), numTexels);
        textureColor = sampleNeighborhood(samples, _numChannels, _textureSize, nearestTexel);
    }
    case CoPixelFormat::Invalid :
    default : break;
    }

    return textureColor;
}

std::shared_ptr<CoTexture> CoByteTexture::create(const CreateFromBytesInfo &createInfo) {
    if (!checkCreateInfo(createInfo)) {
        return nullptr;
    }

    return std::shared_ptr<CoTexture>(new CoByteTexture(createInfo));
}

} // namespace cblt::render
