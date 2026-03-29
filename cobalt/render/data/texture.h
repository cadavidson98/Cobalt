#ifndef COBALT_RENDER_TEXTURE_H
#define COBALT_RENDER_TEXTURE_H

#include "color/pixel_buffer.h"
#include "color/polynomial_spectrum.h"
#include "math/math_types.h"
#include "rgb2spec/rgb2spec.h"

namespace cobalt::render {

class Texture {
public:
    struct CreateInfo {
        std::shared_ptr<color::PixelBuffer> pixelBuffer;
    };

    static std::shared_ptr<Texture> create(const CreateInfo &createInfo);
    ~Texture();

    color::PolynomialSpectrum sample(vec2f coordinates);

private:
    Texture() = delete;
    Texture(std::shared_ptr<color::PixelBuffer> pixelBuffer);

    std::shared_ptr<color::PixelBuffer> _pixelBuffer;
    RGB2Spec *_rgbToSpecLUT;
};

} // namespace cobalt::render

#endif // COBALT_RENDER_TEXTURE_H
