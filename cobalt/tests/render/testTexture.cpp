#include "render/data/texture.h"

#include <gtest/gtest.h>

#include <cstring>

TEST(CobaltRenderDataTests, TestReadTexture) {
    std::shared_ptr<float[]> textureData = std::make_shared<float[]>(4);

    textureData[0] = 0.f;
    textureData[1] = 1.f;
    textureData[2] = 2.f;
    textureData[3] = 3.f;

    std::shared_ptr<cblt::render::CoTexture> texture = cblt::render::CoTexture::create({
        .bytes = textureData,
        .format = cblt::render::CoPixelFormat::Float,
        .numChannels = 1,
        .dimensions = {2, 2},
    });

    EXPECT_TRUE(texture);

    const cblt::vec2u textureSize = texture->size();

    EXPECT_EQ(textureSize.x, 2u);
    EXPECT_EQ(textureSize.y, 2u);
}
