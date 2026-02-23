#include "commands.h"
#include "image_writer.h"

#include "color/pixel_buffer.h"
#include "core/size_types.h"
#include "core/system.h"
#include "render/renderer.h"
#include "render/scene/scene.h"
#include "render/scene/scene_factory.h"

#include <iostream>
#include <memory>
#include <optional>
#include <string>

namespace cobalt::cli {

namespace {

struct Arguments {
    std::string inputFile;
    std::string outputFile;
};

void printUsage() {
    std::cout << "render -i [input file] -o [output file] -c [renderer configuration]";
}

std::optional<Arguments> parseArguments(std::span<char *> args) {
    if (args.size() == 1) {
        printUsage();
        return std::nullopt;
    }

    Arguments arguments;
    for (size_t idx = 0; idx < args.size(); ++idx) {
        const std::string_view command = args[idx];
        if (command == "-i" || command == "--input") {
            arguments.inputFile = args[++idx];
            continue;
        } else if (command == "-o" || command == "--output") {
            arguments.outputFile = args[++idx];
            continue;
        } else {
            return std::nullopt;
        }
    }

    return arguments;
}

} // anonymous namespace

bool renderCommand(std::span<char *> args) {

    const std::optional<Arguments> settings = parseArguments(args);
    if (!settings) {
        printUsage();
        return false;
    }

    const uint64_t loadStart = core::time();

    std::shared_ptr<render::Scene> scene = render::SceneFactory::buildScene({
        .fileName = settings->inputFile,
        .format = render::SceneFactory::SceneFormat::kMitsuba,
    });

    if (!scene) {
        std::cout << "Failed to load scene" << std::endl;
        return false;
    }

    const uint64_t loadEnd = core::time();

    std::cout << "Loaded Scene in " << (loadEnd - loadStart) * 1e-6 << " ms " << std::endl;

    static constexpr uint32_t kWidth = 800;
    static constexpr uint32_t kHeight = 800;
    std::shared_ptr<color::PixelBuffer> pixelBuffer = color::PixelBuffer::create({
        .colorspace = color::rgb::Colorspace::kSRGB,
        .size = {kWidth, kHeight},
    });

    if (!pixelBuffer) {
        std::cerr << "no render target" << std::endl;
        return false;
    }

    const uint64_t renderStartTime = core::time();

    if (!render::render(scene, pixelBuffer)) {
        std::cerr << "Failed to render scene";
        return false;
    }

    const uint64_t renderEndTime = core::time();

    std::cout << "Rendered image in " << (renderEndTime - renderStartTime) * 1e-9 << " s\n";

    const bool wrote = writeImage({
        .fileName = settings->outputFile,
        .type = ImageType::kEXR,
        .pixelBuffer = pixelBuffer,
    });

    if (!wrote) {
        std::cerr << "failed to write" << std::endl;
        return false;
    }

    std::cout << "wrote image " << settings->outputFile << std::endl;
    return true;
}

} // namespace cobalt::cli
