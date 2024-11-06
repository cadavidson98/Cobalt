#include "argument_parser.h"
#include "assets.h"
#include "bounding_box.h"
#include "constants.h"
#include "image.h"
#include "intersection.h"
#include "logging.h"
#include "math_utilities.h"
#include "math_utils.h"
#include "render_target.h"
#include "scene.h"
#include "sphere.h"
#include "texture.h"

#include <cstdlib>
#include <iostream>

CBLT_DEFINE_LOG(CobaltCLI);

int main(int argc, char *argv[]) {
    std::optional<cblt::cli::CoCLIParams> settings = cblt::cli::parseArguments(argc, argv);

    if (!settings) {
        CoLogError(CobaltCLI) << "no settings";
        return EXIT_FAILURE;
    }

    static constexpr uint32_t kWidth = 400;
    static constexpr uint32_t kHeight = 400;
    std::shared_ptr<cblt::render::CoRenderTarget> renderTarget = cblt::render::CoRenderTarget::create({
        .size = {kWidth, kHeight},
    });

    static const cblt::render::CoCamera kDefaultCamera(cblt::render::CoCamera::CreateFromProjectionInfo{
        .hFov = cblt::toRadians(40.f),
        .vFov = cblt::toRadians(40.f),
        .filmSize = cblt::CoSize(2.2f, 2.2f),
        .cameraToWorld = cblt::mat4f(1.f),
    });

    std::shared_ptr<cblt::render::CoTexture> envMap = cblt::render::CoTexture::Create({
        .fileName = cblt::tools::asset::kAssetsTexturesDir + "sky.exr",
        .fileExtension = "exr",
    });

    cblt::render::CoScene::CreateFromDataInfo createInfo{
        .camera = kDefaultCamera,
        .environmentMap = envMap,
    };

    std::shared_ptr<cblt::render::CoScene> defaultScene = cblt::render::CoScene::Create(createInfo);

    const cblt::geom::CoSphere testSphere{
        .center = cblt::simd::vec3f(1.f, 0.f, 15.f),
        .radius = 2.f,
    };

    static const cblt::CoRect viewport = cblt::CoRect{
        .offset = {0.f, 0.f},
        .size = cblt::CoSize(kWidth, kHeight),
    };

    const cblt::vec2f viewportDimensions = {
        viewport.size.width - viewport.offset.x,
        viewport.size.height - viewport.offset.y,
    };

    auto viewportToNDC = [&viewportDimensions](cblt::vec2f pixelPos) {
        return ((pixelPos / viewportDimensions) * cblt::vec2f{2.f, -2.f} + cblt::vec2f{-1.f, 1.f});
    };

    for (uint32_t pixelX = 0; pixelX < kWidth; ++pixelX) {
        for (uint32_t pixelY = 0; pixelY < kHeight; ++pixelY) {
            float tMin{0.f}, tMax{0.f};
            const cblt::geom::CoRay ray = kDefaultCamera.CreateRay(viewportToNDC({float(pixelX), float(pixelY)}));
            const bool hit = cblt::geom::raySphereIntersection(ray, testSphere, tMin, tMax);
            renderTarget->Write({pixelX, pixelY}, {float(hit), 0.f, float(hit), 1.f});
        }
    }

    if (!renderTarget) {
        CoLogError(CobaltCLI) << "no render target";
        return EXIT_FAILURE;
    }

    std::shared_ptr<cblt::cli::Image> renderImage = cblt::cli::Image::create({
        .renderTarget = renderTarget,
    });

    if (!renderImage) {
        CoLogError(CobaltCLI) << "no image";
        return EXIT_FAILURE;
    }

    const bool wrote = renderImage->write({
        .filePath = settings->outputFile,
        .extension = "png",
    });

    if (!wrote) {
        CoLogError(CobaltCLI) << "failed to write";
        return EXIT_FAILURE;
    }

    CoLogDebug(CobaltCLI) << "wrote image", settings->outputFile;
    return EXIT_SUCCESS;
}
