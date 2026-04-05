#include "color/pixel_buffer.h"
#include "color/rgb.h"
#include "math/vec2.h"
#include "render/data/camera.h"
#include "render/data/texture.h"

#include <gtest/gtest.h>

namespace cobalt::render {

namespace {

std::shared_ptr<color::PixelBuffer> makeCheckerboard() {
    static constexpr uint32_t kWidth = 16;
    static constexpr uint32_t kHeight = 16;

    std::shared_ptr<color::PixelBuffer> checkerboard = color::PixelBuffer::create({
        .colorspace = color::rgb::Colorspace::kSRGB,
        .size = vec2u{
                      .x = kWidth,
                      .y = kHeight,
                      },
    });

    for (uint32_t y = 0; y < kHeight; ++y) {
        for (uint32_t x = 0; x < kWidth; ++x) {
            const vec2u idx = {
                .x = x,
                .y = y,
            };

            const float color = float((y * kWidth + x) & 0x01);

            checkerboard->at(idx) = color::rgb::Value{
                .r = color,
                .g = color,
                .b = color,
            };
        }
    }

    return checkerboard;
}

} // anonymous namespace

TEST(CobaltRender, TestCamera) {
    // camera is also 'out of date'
    // need to review...
}

TEST(CobaltRender, TestTexture) {
    {
        // missing pixel buffer
        std::shared_ptr<Texture> texture = Texture::create({.pixelBuffer = nullptr});

        EXPECT_FALSE(texture);
    }

    std::shared_ptr<Texture> texture = Texture::create({
        .pixelBuffer = makeCheckerboard(),
    });

    ASSERT_TRUE(texture);
    // not sure if I should UT sample... I think it suggests I need to 'expose' more sampling behavior
    // filter size, clamping behavior, never fails
    // texture->sample();
}

} // namespace cobalt::render
