#include "render.h"

#include "assets.h"
#include "cli_progress.h"
#include "commands.h"
#include "image.h"

#include "core/callback.h"
#include "core/size_types.h"
#include "core/system.h"
#include "geometry/ray.h"
#include "render/data/color.h"
#include "render/data/render_target.h"
#include "render/scene/scene.h"
#include "render/scene/scene_factory.h"

#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

namespace cblt::cli {

namespace {
void printUsage() {
    std::cout << "render -i [input file] -o [output file] -c [renderer configuration]";
}

bool loadConfiguration(const std::string &file_path, RenderConfiguration &settings, RenderTarget &outputImageTarget) {
    std::ifstream fin(file_path);
    if (!fin.good()) {
        return false;
    }

    std::string line;
    while (!fin.eof()) {
        fin >> line;
        if (!line.compare("width:")) {
            fin >> outputImageTarget.width;
        } else if (!line.compare("height:")) {
            fin >> outputImageTarget.height;
        } else if (!line.compare("path_depth:")) {
            fin >> settings.maxPathDepth;
        } else if (!line.compare("num_samples:")) {
            fin >> settings.samplesPerPixel;
        } else if (!line.compare("tile_size:")) {
            fin >> settings.tileSize;
        } else if (!line.compare("num_threads:")) {
            fin >> settings.numThreads;
        }
    }

    return true;
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
            // check for extension
            size_t extensionPos = inputFileName.find_last_of('.');
            assert(extensionPos != std::string::npos);
            std::string fileType = inputFileName.substr(extensionPos);
            continue;
        } else if (command.compare("-o") == 0U || command.compare("--output") == 0U) {
            params.outputFile = argv[++arg];
            continue;
        } else if (command.compare("-c") == 0U || command.compare("--configuration") == 0U) {
            RenderConfiguration fileConfiguration;
            RenderTarget fileRenderTarget;
            std::string inputConfigFile = argv[++arg];
            if (loadConfiguration(inputConfigFile, fileConfiguration, fileRenderTarget)) {
                params.outputImageTarget.emplace(fileRenderTarget);
                params.runtimeSettings.emplace(fileConfiguration);
            }
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
    std::mutex progressMutex;
    core::CoCallback progressCallback;
    progressCallback.functor = [&progressMutex](const char *message, int totalProgress) {
        std::scoped_lock(progressMutex);
        printProgress(totalProgress, message);
    };

    std::shared_ptr<render::CoScene> defaultScene = render::CoSceneFactory::buildScene(
        {
            .fileName = tools::asset::kAssetsBaseDir + "teapot.xml",
            .parentDirectory = tools::asset::kAssetsBaseDir,
            .format = render::CoSceneFactory::SceneFormat::kMitsuba,
        },
        progressCallback
    );

    if (!defaultScene) {
        return false;
    }

    const uint64_t loadEnd = core::time();
    // report load time
    std::cout << "Loaded Scene in " << (loadEnd - loadStart) * 1e-6 << " ms " << std::endl;

    static constexpr uint32_t kWidth = 800;
    static constexpr uint32_t kHeight = 800;
    std::shared_ptr<render::CoRenderTarget> renderTarget = render::CoRenderTarget::create({
        .size = {
                 kWidth, kHeight,
                 },
    });

    if (!renderTarget) {
        std::cerr << "no render target" << std::endl;
        return false;
    }

    const vec2f viewportDimensions = {
        float(kWidth),
        float(kHeight),
    };

    auto viewportToNDC = [&viewportDimensions](vec2f pixelPos) {
        return ((pixelPos / viewportDimensions) * vec2f{2.f, -2.f} + vec2f{-1.f, 1.f});
    };

    int renderProgress = 0;
    auto renderCallback = [&progressMutex, &renderProgress](uint32_t currentProgress) {
        std::scoped_lock(progressMutex);
        renderProgress += currentProgress;
        printProgress(renderProgress);
    };

    std::shared_ptr<render::CoCamera> camera = defaultScene->camera();

    const uint32_t numTotalPixels = kWidth * kHeight;
    const uint32_t pumpValue = 5;
    const uint32_t pixelProgress = numTotalPixels / 20;
    const uint64_t renderStartTime = core::time();
    for (uint32_t pixelY = 0; pixelY < kHeight; ++pixelY) {
        for (uint32_t pixelX = 0; pixelX < kWidth; ++pixelX) {
            geom::IntersectionEvent intersectionEvent;
            const geom::CoRay ray = camera->createRay(viewportToNDC({float(pixelX), float(pixelY)}));
            const bool hitMesh = defaultScene->closestIntersection(ray, intersectionEvent);
            if (hitMesh) {
                const render::CoSurfaceParams surfaceParams =
                    defaultScene->resolveSurfaceAtInteraction(intersectionEvent);

                renderTarget->Write({pixelX, pixelY}, surfaceParams.baseColor);
            } else {
                const render::CoColor environmentColor = defaultScene->environment(ray);
                renderTarget->Write(
                    {pixelX, pixelY},
                    {
                        environmentColor.r,
                        environmentColor.g,
                        environmentColor.b,
                        1.f,
                    }
                );
            }
            const uint32_t currentPixel = pixelY * kWidth + pixelX;
            if (currentPixel % pixelProgress == 0) {
                renderCallback(pumpValue);
            }
        }
    }
    const uint64_t renderEndTime = core::time();

    std::cout << "Rendered image in " << (renderEndTime - renderStartTime) * 1e-9 << " s\n";

    std::shared_ptr<Image> renderImage = Image::create({
        .renderTarget = renderTarget,
    });

    if (!renderImage) {
        std::cerr << "no image" << std::endl;
        return false;
    }

    const bool wrote = renderImage->write({
        .filePath = settings->outputFile,
        .extension = "png",
    });

    if (!wrote) {
        std::cerr << "failed to write" << std::endl;
        return false;
    }

    std::cout << "wrote image " << settings->outputFile << std::endl;
    return true;
}

} // namespace cblt::cli
