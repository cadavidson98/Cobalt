#ifndef CBLT_RENDER_BYTE_TEXTURE_H
#define CBLT_RENDER_BYTE_TEXTURE_H

#include "texture.h"

#include "core/size_types.h"

#include <memory>

namespace cblt::render {

class CoByteTexture : public CoTexture {
public:
    struct CreateFromBytesInfo {
        std::shared_ptr<const void> bytes;
        CoPixelFormat format;
        uint32_t numChannels;
        vec2u dimensions;
    };

    static std::shared_ptr<CoTexture> create(const CreateFromBytesInfo &createInfo);

    virtual ~CoByteTexture();

    CoPixelFormat format() const override;
    vec2u size() const override;
    size_t size_bytes() const override;

    CoColor sample(const vec2f &uvCoord) const override;

private:
    CoByteTexture() = delete;
    CoByteTexture(const CreateFromBytesInfo &createInfo);

    std::shared_ptr<const void> _textureData;

    vec2u _textureSize;

    CoPixelFormat _textureFormat;
    uint32_t _numChannels;
};

} // namespace cblt::render

#endif // CBLT_RENDER_BYTE_TEXTURE_H
