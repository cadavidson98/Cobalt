#ifndef CBLT_RENDER_TEXTURE_H
#define CBLT_RENDER_TEXTURE_H

#include "color.h"

#include "core/size_types.h"
#include "math/vec2.h"

#include <Imath/half.h>

#include <memory>

namespace cblt::render {

enum class CoPixelFormat {
    Invalid = 0,
    Float,
    Half,
};

class CoTexture {
public:
    struct CreateFromBytesInfo {
        std::shared_ptr<const void> bytes;
        CoPixelFormat format;
        uint32_t numChannels;
        vec2u dimensions;
    };

    static std::shared_ptr<CoTexture> create(const CreateFromBytesInfo &createInfo);

    ~CoTexture();

    vec2u size() const;
    CoColor sample(const vec2f &uvCoord) const;

private:
    CoTexture() = delete;
    CoTexture(const CreateFromBytesInfo &createInfo);

    std::shared_ptr<const void> _textureData;

    vec2u _textureSize;

    CoPixelFormat _textureFormat;
    uint32_t _numChannels;
};

} // namespace cblt::render

#endif // CBLT_RENDER_TEXTURE_H
