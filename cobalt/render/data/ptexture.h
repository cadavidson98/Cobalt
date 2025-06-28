#ifndef CBLT_RENDER_PTEXTURE_H
#define CBLT_RENDER_PTEXTURE_H

#include "texture.h"

namespace cblt::render {

class CoPTexture : public CoTexture {
public:
    CoPixelFormat format() const override;
    vec2u size() const override;
    size_t size_bytes() const override;

    CoColor sample(const vec2f &uvCoord) const override;

private:
};

} // namespace cblt::render

#endif // CBLT_RENDER_PTEXTURE_H
