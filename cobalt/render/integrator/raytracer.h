#ifndef CBLT_RENDER_RAYTRACER_H
#define CBLT_RENDER_RAYTRACER_H

#include "render_target.h"

#include "math/math_types.h"

#include <functional>
#include <memory>
#include <optional>

namespace cblt::render {

class CoCamera;
class CoScene;

class Raytracer {
public:
    struct CreateInfo {
        // todo: sampler
        // todo: draw mode (depth, color, spectral)
        // todo: render target(s) format
        vec2u renderTargetSize;
    };

    struct DrawOptions {
        uint32_t samplesPerPixel;
        uint32_t maxDepth;
    };

    struct DrawInfo {
        std::reference_wrapper<CoCamera> camera;
        std::reference_wrapper<CoScene> scene;
        DrawOptions drawOptions;
        std::optional<std::function<void(float)>> progressCallback;
    };

    struct RenderTargets {
        std::reference_wrapper<CoRenderTarget> colorAttachment;
        std::optional<std::reference_wrapper<CoRenderTarget>> depthAttachment;
    };

    static std::shared_ptr<Raytracer> create(const CreateInfo &createInfo);

    ~Raytracer();

    std::optional<RenderTargets> render(const DrawInfo &drawInfo);

private:
    Raytracer();

    static constexpr uint32_t kTileSize = 32u;

    static bool _checkCreateInfo(const CreateInfo &createInfo);
    static bool _checkDrawInfo(const DrawInfo &drawInfo);

    void _renderTile(const DrawInfo &drawInfo, vec2u tileIdx);

    vec2u _renderTargetSize;
};

} // namespace cblt::render

#endif // CBLT_RENDER_RAYTRACER_H
