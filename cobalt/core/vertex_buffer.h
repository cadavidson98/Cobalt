#ifndef CBLT_CORE_VERTEX_BUFFER_H
#define CBLT_CORE_VERTEX_BUFFER_H

#include "size_types.h"

#include "math/math_types.h"

#include <memory>

namespace cblt::core {

template<typename T>
struct VertexAttributeBuffer {
    std::shared_ptr<T[]> vertices;
    size_t vertexCount;
    std::shared_ptr<vec3u[]> triangleIndices;
    size_t triangleCount;
    std::shared_ptr<vec4u[]> patchIndices;
    size_t patchCount;
};

} // namespace cblt::core

#endif // CBLT_CORE_VERTEX_BUFFER_H
