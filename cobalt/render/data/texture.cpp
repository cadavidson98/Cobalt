#include "texture.h"

#include "texture_utilities.h"

#include "color/pixel_buffer.h"
#include "color/polynomial_spectrum.h"
#include "color/rgb.h"
#include "core/logging.h"
#include "math/math_types.h"
#include "rgb2spec/rgb2spec.h"

#include <cassert>
#include <memory>

namespace cobalt::render {

namespace {

[[nodiscard]] bool checkCreateInfo(const Texture::CreateInfo &createInfo) {
    if (!createInfo.pixelBuffer) {
        CoLogError("PixelBuffer must be not null");
        return false;
    }

    switch (createInfo.pixelBuffer->colorspace()) {
    case color::rgb::Colorspace::kSRGB :
        return true;
    case color::rgb::Colorspace::kDCIP3 :
        [[fallthrough]];
    default :
        CoLogError("Unsupported colorspace");
        return false;
    }

    return false;
}

} // anonymous namespace

Texture::Texture(std::shared_ptr<color::PixelBuffer> pixelBuffer): _pixelBuffer{pixelBuffer} {

    // todo: needs to support other types
    _rgbToSpecLUT = rgb2spec_load(RGB2SPEC_COLOR_SRGB);
}

std::shared_ptr<Texture> Texture::create(const CreateInfo &createInfo) {
    if (!checkCreateInfo(createInfo)) {
        return nullptr;
    }

    return std::shared_ptr<Texture>(new Texture(createInfo.pixelBuffer));
}

Texture::~Texture() {
    rgb2spec_free(_rgbToSpecLUT);
}

color::PolynomialSpectrum Texture::sample(vec2f coordinates) {
    static constexpr size_t kFilterSize = 3;
    static constexpr float kHalfFilter = kFilterSize >> 1;

    if (std::clamp(coordinates.x, 0.f, 1.f) != coordinates.x || std::clamp(coordinates.y, 0.f, 1.f) != coordinates.y) {
        assert(false);
        return {};
    }

    const vec2f pixelBufferSize = {
        .x = float(_pixelBuffer->size().x),
        .y = float(_pixelBuffer->size().y),
    };

    const vec2f pixelCoordinates = {
        .x = coordinates.x * float(pixelBufferSize.x),
        .y = coordinates.y * float(pixelBufferSize.y),
    };

    vec3f rgbValue = {
        .x = 0.f,
        .y = 0.f,
        .z = 0.f,
    };

    const uint32_t minY = std::max(pixelCoordinates.y - kHalfFilter, 0.f);
    const uint32_t maxY = std::min(pixelCoordinates.y + kHalfFilter, pixelBufferSize.y - 1);

    const uint32_t minX = std::max(pixelCoordinates.x - kHalfFilter, 0.f);
    const uint32_t maxX = std::min(pixelCoordinates.x + kHalfFilter, pixelBufferSize.x - 1);

    for (uint32_t y = minY; y <= maxY; y++) {
        for (uint32_t x = minX; x <= maxX; x++) {
            const vec2f sampleCoordinate = {
                .x = float(x),
                .y = float(y),
            };

            const color::rgb::Value pixel = _pixelBuffer->at({
                .x = x,
                .y = y,
            });

            const vec3f pixelValue = {
                .x = pixel.r,
                .y = pixel.g,
                .z = pixel.b,
            };

            rgbValue += tentFilter(sampleCoordinate, pixelCoordinates, pixelValue);
        }
    }

    std::array<float, 3> rgb = {rgbValue.x, rgbValue.y, rgbValue.z};

    color::PolynomialSpectrum spectrum;

    rgb2spec_fetch(_rgbToSpecLUT, rgb.data(), spectrum.coefficients.data());

    return spectrum;
}

} // namespace cobalt::render
