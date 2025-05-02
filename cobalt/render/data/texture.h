#ifndef CBLT_RENDER_TEXTURE_H
#define CBLT_RENDER_TEXTURE_H

#include "color.h"

#include "core/size_types.h"
#include "math/math_types.h"

#include <Imath/half.h>

namespace cblt::render {

enum class CoPixelFormat {
    Invalid = 0,
    Float,
    Half,
};

class CoTexture {
public:
    virtual ~CoTexture() = default;

    virtual CoPixelFormat format() const = 0;
    virtual vec2u size() const = 0;
    virtual size_t size_bytes() const = 0;

    virtual CoColor sample(const vec2f &uvCoord) const = 0;
};

} // namespace cblt::render

#endif // CBLT_RENDER_TEXTURE_H
