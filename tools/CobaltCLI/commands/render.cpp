#include "render.h"

#include "assets.h"
#include "commands.h"
#include "image_writer.h"

#include "core/size_types.h"
#include "core/system.h"
#include "render/data/render_target.h"
#include "render/renderer.h"
#include "render/scene/scene.h"
#include "render/scene/scene_factory.h"

#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

namespace cblt::cli {

namespace {
void printUsage() {
    std::cout << "render -i [input file] -o [output file] -c [renderer configuration]";
}

std::optional<CoCLIParams> parseArguments(int numArgs, char **argv) {
    if (numArgs == 1) {
        printUsage();
        return std::nullopt;
    }

    CoCLIParams params;
    for (int arg = 1; arg < numArgs; ++arg) {
        std::string command(argv[arg]);
        if (command.compare("-i") == 0U || command.compare("--input") == 0U) {
            std::string inputFileName = argv[++arg];
            params.inputFile = inputFileName;
            continue;
        } else if (command.compare("-o") == 0U || command.compare("--output") == 0U) {
            params.outputFile = argv[++arg];
            continue;
        } else {
            printUsage();
            return std::nullopt;
        }
    }
    return params;
}

} // anonymous namespace

bool renderCommand(int argc, char **argv) {

    const std::optional<CoCLIParams> settings = parseArguments(argc - 1, argv + 1);
    if (!settings) {
        return false;
    }

    const uint64_t loadStart = core::time();

    std::shared_ptr<render::CoScene> scene = render::CoSceneFactory::buildScene({
        .fileName = settings->inputFile,
        .format = render::CoSceneFactory::SceneFormat::kMitsuba,
    });

    if (!scene) {
        std::cout << "Failed to load scene" << std::endl;
        return false;
    }

    const uint64_t loadEnd = core::time();
    // report load time
    std::cout << "Loaded Scene in " << (loadEnd - loadStart) * 1e-6 << " ms " << std::endl;

    static constexpr uint32_t kWidth = 800;
    static constexpr uint32_t kHeight = 800;
    std::shared_ptr<render::CoRenderTarget> renderTarget = render::CoRenderTarget::create({
        .size = {kWidth, kHeight},
    });

    if (!renderTarget) {
        std::cerr << "no render target" << std::endl;
        return false;
    }

    const uint64_t renderStartTime = core::time();

    if (!render::render(*scene, renderTarget)) {
        std::cerr << "Failed to render image";
        return false;
    }

    const uint64_t renderEndTime = core::time();

    std::cout << "Rendered image in " << (renderEndTime - renderStartTime) * 1e-9 << " s\n";

    const bool wrote = writeImage({
        .fileName = settings->outputFile,
        .type = ImageType::kEXR,
        .renderTarget = *renderTarget,
    });

    if (!wrote) {
        std::cerr << "failed to write" << std::endl;
        return false;
    }

    std::cout << "wrote image " << settings->outputFile << std::endl;
    return true;
}

} // namespace cblt::cli
