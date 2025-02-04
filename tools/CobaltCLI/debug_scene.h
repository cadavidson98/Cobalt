#ifndef CBLT_RENDER_SCENE_DEBUG_BUILDER_H
#define CBLT_RENDER_SCENE_DEBUG_BUILDER_H

#include <memory>
#include <functional>

namespace cblt::render {
    class CoScene;
}  // namespace cblt

namespace cblt::tools {

std::shared_ptr<render::CoScene> debugScene(core::CoCallback &callback);

} // namespace cblt::tools

#endif // CBLT_RENDER_SCENE_DEBUG_RENDER_H
