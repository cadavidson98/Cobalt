#ifndef CBLT_IO_OBJ_READER_H
#define CBLT_IO_OBJ_READER_H

#include "core/vertex_buffer.h"
#include "math/math_types.h"

#include <memory>
#include <optional>
#include <string_view>

namespace cblt::io::obj {

struct Mesh {
    core::VertexAttributeBuffer<simd::vec3f> positions;
    core::VertexAttributeBuffer<vec2f> uvs;
    core::VertexAttributeBuffer<simd::vec3f> normals;
};

std::optional<Mesh> read(const std::string_view fileName);

} // namespace cblt::io::obj

#endif // CBLT_IO_OBJ_READER_H
