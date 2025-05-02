#ifndef CBLT_RENDER_OBJ_UTILS_H
#define CBLT_RENDER_OBJ_UTILS_H

#include <memory>
#include <string>

namespace cblt::geom {
class CoMesh;
} // namespace cblt::geom

namespace cblt::render::utils {

std::shared_ptr<geom::CoMesh> readObjFile(const std::string &fileName);

} // namespace cblt::render::utils

#endif // CBLT_RENDER_OBJ_UTILS_H
