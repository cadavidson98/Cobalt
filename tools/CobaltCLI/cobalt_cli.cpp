#include "argument_parser.h"
#include "assets.h"
#include "constants.h"
#include "image.h"
#include "intersection.h"
#include "logging.h"
#include "math_utilities.h"
#include "math_utils.h"
#include "mesh.h"
#include "quad.h"
#include "render_target.h"
#include "scene.h"
#include "sphere.h"
#include "texture.h"
#include "triangle.h"

#include <cstdlib>

CBLT_DEFINE_LOG(CobaltCLI);

int main(int argc, char *argv[]) {
    std::optional<cblt::cli::CoCLIParams> settings = cblt::cli::parseArguments(argc, argv);

    if (!settings) {
        CoLogError(CobaltCLI) << "no settings";
        return EXIT_FAILURE;
    }

    static const cblt::render::CoCamera kDefaultCamera(cblt::render::CoCamera::CreateFromProjectionInfo{
        .hFov = cblt::toRadians(40.f),
        .vFov = cblt::toRadians(40.f),
        .filmSize = cblt::CoSize(2.2f, 2.2f),
        .cameraToWorld =
            cblt::mat4f{
                {1.f, 0.f, 0.f, 0.f},
                {0.f, 1.f, 0.f, 0.f},
                {0.f, 0.f, 1.f, 0.f},
                {0.f, 0.f, -5.f, 1.f},
            },
    });

    std::shared_ptr<cblt::render::CoTexture> envMap = cblt::render::CoTexture::create({
        .fileName = cblt::tools::asset::kAssetsTexturesDir + "arches.exr",
        .fileExtension = "exr",
    });

    std::shared_ptr<cblt::geom::CoMesh> teapotMesh = cblt::geom::CoMesh::create({
        .fileName = cblt::tools::asset::kAssetsMeshesDir + "teapot/teapot.obj",
        .fileExtension = "obj",
    });

    cblt::render::CoScene::CreateFromDataInfo createInfo{
        .camera = kDefaultCamera,
        .environmentMap = envMap,
    };

    std::shared_ptr<cblt::render::CoScene> defaultScene = cblt::render::CoScene::Create(createInfo);

    std::shared_ptr<cblt::geom::CoMesh> testMesh = cblt::geom::CoMesh::create({
        .fileName = cblt::tools::asset::kAssetsMeshesDir + std::string("teapot/teapot.obj"),
        .fileExtension = "obj",
    });

    static constexpr uint32_t kWidth = 800;
    static constexpr uint32_t kHeight = 800;
    std::shared_ptr<cblt::render::CoRenderTarget> renderTarget = cblt::render::CoRenderTarget::create({
        .size =
            {
                kWidth,
                kHeight,
            },
    });

    static const cblt::CoRect viewport = cblt::CoRect{
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

    const cblt::vec2f viewportDimensions = {
        viewport.size.width - viewport.offset.x,
        viewport.size.height - viewport.offset.y,
    };

    auto viewportToNDC = [&viewportDimensions](cblt::vec2f pixelPos) {
        return ((pixelPos / viewportDimensions) * cblt::vec2f{2.f, -2.f} + cblt::vec2f{-1.f, 1.f});
    };

    {
        cblt::geom::IntersectionEvent intersectionEvent;
        const cblt::geom::CoRay ray = kDefaultCamera.CreateRay(viewportToNDC({400.f, 250.f}));
        const bool hitMesh = testMesh->intersects(ray, intersectionEvent);
        CoLogDebug(CobaltCLI) << "Hit Mesh:" << hitMesh;
    }

    for (uint32_t pixelY = 0; pixelY < kHeight; ++pixelY) {
        for (uint32_t pixelX = 0; pixelX < kWidth; ++pixelX) {
            cblt::geom::IntersectionEvent intersectionEvent;
            const cblt::geom::CoRay ray = kDefaultCamera.CreateRay(viewportToNDC({float(pixelX), float(pixelY)}));
            const bool hitMesh = testMesh->intersects(ray, intersectionEvent);
            if (hitMesh) {
                const float depth = (intersectionEvent.timeMin - 4.f) / 2.f;
                const float u = intersectionEvent.localCoordinates.x;
                const float v = intersectionEvent.localCoordinates.y;
                renderTarget->Write({pixelX, pixelY}, {u, 0.f, v, 1.f});
            } else {
                const float phi = std::acos(ray.dir.y);
                // TODO: make sure camera is using an rhs csys
                float theta = std::atan2(-ray.dir.z, ray.dir.x);
                theta = (theta < 0.f) ? theta + cblt::kPI : theta;
                const float u = ((theta) / (2.f * cblt::kPI));
                const float v = phi / cblt::kPI;

                const cblt::render::CoColor envColor = envMap->sample({u, v});
                renderTarget->Write({pixelX, pixelY}, {envColor.r, envColor.g, envColor.b, envColor.a});
            }
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
