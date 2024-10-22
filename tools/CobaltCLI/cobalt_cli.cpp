#include "argument_parser.h"
#include "image.h"
#include "logging.h"
#include "render_target.h"

#include <cstdlib>
#include <iostream>

CBLT_DEFINE_LOG(CobaltCLI);

int main(int argc, char* argv[]) {
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

    for (uint32_t pixelX = 0; pixelX < kWidth; ++pixelX) {
        for (uint32_t pixelY = 0; pixelY < kHeight; ++pixelY) {
                renderTarget->Write({pixelX, pixelY}, {0.f, 0.f, 1.f, 1.f});
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

    bool wrote = renderImage->write({
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