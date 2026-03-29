#ifndef COBALT_RENDER_SCENE_BUILDER_H
#define COBALT_RENDER_SCENE_BUILDER_H

#include "math/math_types.h"

#include <filesystem>
#include <memory>

namespace cobalt::render {

class Scene;
class Material;

class SceneBuilder {
public:
    enum class SceneFormat {
        kMitsuba,
    };

    struct CreateInfo {
        std::filesystem::path fileName;
        SceneFormat format;
    };

    static std::shared_ptr<Scene> buildScene(const CreateInfo &createInfo);
};

} // namespace cobalt::render

#endif // COBALT_RENDER_SCENE_BUILDER_H
