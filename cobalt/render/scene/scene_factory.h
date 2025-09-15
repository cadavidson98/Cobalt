#ifndef CBLT_RENDER_SCENE_BUILDER_H
#define CBLT_RENDER_SCENE_BUILDER_H

#include "math/math_types.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

namespace cblt::render {

class CoScene;
class CoMaterial;

class CoSceneFactory {
public:
    enum class SceneFormat {
        kMitsuba,
    };

    enum class BuildingError {
        kMissingFile,     // Failed to open the scene file, or a dependency
        kParsingError,    // Failed to read the contents of a file
        kInvalidArgument, // Failed to convert data to the Cobalt format
    };

    struct CreateInfo {
        std::filesystem::path fileName;
        SceneFormat format;
    };

    static std::shared_ptr<CoScene> buildScene(const CreateInfo &createInfo);
};

} // namespace cblt::render

#endif // CBLT_RENDER_SCENE_BUILDER_H
