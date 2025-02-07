#ifndef CBLT_RENDER_MITSUBA_UTILS_H
#define CBLT_RENDER_MITSUBA_UTILS_H

#include "callback.h"
#include "surface_params.h"

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace cblt::geom {

class CoMesh;

} // namespace cblt::geom

namespace cblt::render {

class CoCamera;
class CoTexture;

namespace utils {

struct MitsubaTexture {
    std::string fileName;
};

using MitsubaMaterial = CoPrincipledParams<std::variant<vec4f, MitsubaTexture>, float>;

struct MitsubaScene {
    std::shared_ptr<CoCamera> camera;
    std::vector<MitsubaMaterial> materials;
    std::vector<std::shared_ptr<geom::CoMesh>> meshes;
    std::shared_ptr<CoTexture> environmentMap;
};

std::optional<MitsubaScene>
readMitsuba(const std::string &fileName, const std::string &parentDirectory, core::CoCallback &callback);

} // namespace utils
} // namespace cblt::render

#endif // CBLT_RENDER_MITSUBA_UTILS_H
