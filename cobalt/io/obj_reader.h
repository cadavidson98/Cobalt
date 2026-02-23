#ifndef COBALT_IO_OBJ_READER_H
#define COBALT_IO_OBJ_READER_H

#include "core/vertex_buffer.h"
#include "math/math_types.h"

#include <memory>
#include <optional>
#include <string_view>

namespace cobalt::io::obj {

struct Mesh {
    core::VertexAttributeBuffer<simd::vec3f> positions;
    core::VertexAttributeBuffer<vec2f> uvs;
    core::VertexAttributeBuffer<simd::vec3f> normals;
};

std::optional<Mesh> read(const std::string_view fileName);

} // namespace cobalt::io::obj

#endif // COBALT_IO_OBJ_READER_H
