#ifndef COBALT_RENDER_RENDERER_H
#define COBALT_RENDER_RENDERER_H

#include <memory>

namespace cobalt {

namespace color {

class PixelBuffer;

} // namespace color

namespace render {

class Scene;

[[nodiscard]] bool render(std::shared_ptr<const Scene> scene, std::shared_ptr<color::PixelBuffer> pixelBuffer);

} // namespace render

} // namespace cobalt

#endif // COBALT_RENDER_RENDERER_H
