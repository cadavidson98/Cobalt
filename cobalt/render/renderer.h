#ifndef CBLT_RENDER_RENDERER_H
#define CBLT_RENDER_RENDERER_H

#include <memory>

namespace cblt::render {

class CoScene;
class CoRenderTarget;

[[nodiscard]] bool render(const CoScene &scene, std::shared_ptr<CoRenderTarget> renderTarget);

} // namespace cblt::render

#endif // CBLT_RENDER_RENDERER_H
