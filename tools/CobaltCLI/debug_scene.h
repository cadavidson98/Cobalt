#ifndef CBLT_RENDER_SCENE_DEBUG_BUILDER_H
#define CBLT_RENDER_SCENE_DEBUG_BUILDER_H

#include <functional>
#include <memory>

namespace cblt::render {
class CoScene;
} // namespace cblt::render

namespace cblt::tools {

std::shared_ptr<render::CoScene> debugScene(core::CoCallback &callback);

} // namespace cblt::tools

#endif // CBLT_RENDER_SCENE_DEBUG_RENDER_H
