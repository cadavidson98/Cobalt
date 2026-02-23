#ifndef COBALT_CORE_VERTEX_BUFFER_H
#define COBALT_CORE_VERTEX_BUFFER_H

#include "size_types.h"

#include "math/math_types.h"

#include <memory>

namespace cobalt::core {

template<typename T>
struct VertexAttributeBuffer {
    std::shared_ptr<T[]> vertices;
    size_t vertexCount;
    std::shared_ptr<vec3u[]> triangleIndices;
    size_t triangleCount;
    std::shared_ptr<vec4u[]> patchIndices;
    size_t patchCount;
};

} // namespace cobalt::core

#endif // COBALT_CORE_VERTEX_BUFFER_H
