#ifndef CBLT_RENDER_RENDERER_H
#define CBLT_RENDER_RENDERER_H

#include <memory>

namespace cblt {

namespace color {

class PixelBuffer;

} // namespace color

namespace render {

class Scene;

[[nodiscard]] bool render(std::shared_ptr<const Scene> scene, std::shared_ptr<color::PixelBuffer> pixelBuffer);

} // namespace render

} // namespace cblt

#endif // CBLT_RENDER_RENDERER_H
