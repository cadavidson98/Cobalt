#ifndef CBLT_RENDER_SCENE_BUILDER_H
#define CBLT_RENDER_SCENE_BUILDER_H

#include "callback.h"
#include "mesh.h"
#include "dynamic_array.h"
#include "simd/simd_vec3.h"
#include "vec4.h"

#include <memory>
#include <optional>
#include <string>

namespace cblt::render {

class CoScene;
class CoMaterial;

class CoSceneFactory {
    public:
        enum class SceneFormat {
            kObj,
            kMitsuba,
        };

        enum class BuildingError {
            kMissingFile,      // Failed to open the scene file, or a dependency
            kParsingError,     // Failed to read the contents of a file
            kInvalidArgument,  // Failed to convert data to the Cobalt format
        };

        struct CreateInfo {
            std::string fileName;
            std::optional<std::string> parentDirectory;
            SceneFormat format;
        };

        static std::shared_ptr<CoScene> buildScene(const CreateInfo &createInfo, core::CoCallback &callback);

    protected:
        static std::shared_ptr<CoScene> _loadMitsubaScene(const CreateInfo &createInfo, core::CoCallback &callback);
        static std::shared_ptr<CoScene> _loadObjScene(const CreateInfo &createInfo, core::CoCallback &callback);
};

} // namespace cblt::render

#endif // CBLT_RENDER_SCENE_BUILDER_H
