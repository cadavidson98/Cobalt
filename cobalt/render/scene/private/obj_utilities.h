#ifndef CBLT_RENDER_OBJ_UTILS_H
#define CBLT_RENDER_OBJ_UTILS_H

#include "dynamic_array.h"

#include <memory>
#include <optional>
#include <string>

namespace cblt::geom {
class CoMesh;
} // namespace cblt::geom

namespace cblt::render::utils {

std::shared_ptr<geom::CoMesh> readObjFile(const std::string &fileName);

} // namespace cblt::render::utils

#endif // CBLT_RENDER_OBJ_UTILS_H
