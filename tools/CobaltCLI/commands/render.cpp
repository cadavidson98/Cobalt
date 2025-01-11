#include "render.h"

#include "color.h"
#include "commands.h"
#include "debug_builder.h"
#include "image.h"
#include "logging.h"
#include "ray.h"
#include "render_target.h"
#include "scene.h"
#include "size_types.h"

#include <fstream>
#include <string>

CBLT_DEFINE_LOG(CobaltCLIRender);

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

    std::shared_ptr<tools::CoDebugBuilder> sceneBuilder =
        std::shared_ptr<tools::CoDebugBuilder>(new tools::CoDebugBuilder);

    if (!sceneBuilder->buildMeshes() || !sceneBuilder->buildCameras() || !sceneBuilder->buildEnvironment()) {
        return false;
    }

    static const render::CoCamera kDefaultCamera(render::CoCamera::CreateFromProjectionInfo{
        .hFov = toRadians(40.f),
        .vFov = toRadians(40.f),
        .filmSize = CoSize(2.2f, 2.2f),
        .cameraToWorld = utils::translationMatrix({0.f, 0.f, -5.f}),
    });

    std::shared_ptr<render::CoScene> defaultScene = sceneBuilder->scene();
    if (!defaultScene) {
        return false;
    }

    static constexpr uint32_t kWidth = 800;
    static constexpr uint32_t kHeight = 800;
    std::shared_ptr<render::CoRenderTarget> renderTarget = render::CoRenderTarget::create({
        .size =
            {
                kWidth,
                kHeight,
            },
    });

    static const CoRect viewport = CoRect{
        .offset =
            {
                0.f,
                0.f,
            },
        .size =
            {
                kWidth,
                kHeight,
            },
    };

    const vec2f viewportDimensions = {
        viewport.size.width - viewport.offset.x,
        viewport.size.height - viewport.offset.y,
    };

    auto viewportToNDC = [&viewportDimensions](vec2f pixelPos) {
        return ((pixelPos / viewportDimensions) * vec2f{2.f, -2.f} + vec2f{-1.f, 1.f});
    };

    for (uint32_t pixelY = 0; pixelY < kHeight; ++pixelY) {
        for (uint32_t pixelX = 0; pixelX < kWidth; ++pixelX) {
            geom::IntersectionEvent intersectionEvent;
            const geom::CoRay ray = kDefaultCamera.CreateRay(viewportToNDC({float(pixelX), float(pixelY)}));
            const bool hitMesh = defaultScene->closestIntersection(ray, intersectionEvent);
            if (hitMesh) {
                const float depth = (intersectionEvent.timeMin - 4.f) / 2.f;
                const uint32_t idx = intersectionEvent.primitiveIndex;
                vec4f faceColors[4] = {
                    {1.f, 0.f, 0.f, 1.f},
                    {0.f, 0.f, 1.f, 1.f},
                    {0.f, 1.f, 1.f, 1.f},
                    {1.f, 1.f, 1.f, 1.f},
                };
                renderTarget->Write({pixelX, pixelY}, faceColors[idx % 4]);
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
        }
    }

    if (!renderTarget) {
        CoLogError(CobaltCLIRender) << "no render target";
        return false;
    }

    std::shared_ptr<Image> renderImage = Image::create({
        .renderTarget = renderTarget,
    });

    if (!renderImage) {
        CoLogError(CobaltCLIRender) << "no image";
        return false;
    }

    const bool wrote = renderImage->write({
        .filePath = settings->outputFile,
        .extension = "png",
    });

    if (!wrote) {
        CoLogError(CobaltCLIRender) << "failed to write";
        return false;
    }

    CoLogDebug(CobaltCLIRender) << "wrote image " << settings->outputFile;
    return true;
}

} // namespace cblt::cli
