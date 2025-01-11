#include "resources.h"
#include "texture.h"

#include <cstring>
#include <gtest/gtest.h>
#include <iostream>

TEST(CobaltRenderDataTests, TestLoadTextureFromEXR) {
    static const cblt::render::CoTexture::CreateFromFileInfo exrInfo{
        .fileName = std::string(cblt::test::kTestDataDir) + "/sky.exr",
        .fileExtension = "exr",
    };
    std::shared_ptr<cblt::render::CoTexture> texture = cblt::render::CoTexture::create(exrInfo);
    ASSERT_TRUE(texture != nullptr);
}
