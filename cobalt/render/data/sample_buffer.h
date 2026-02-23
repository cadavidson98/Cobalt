#ifndef COBALT_RENDER_RENDERTARGET_H
#define COBALT_RENDER_RENDERTARGET_H

#include "core/size_types.h"
#include "math/math_types.h"

#include <memory>

namespace cobalt::render {

class SampleBuffer {
public:
    struct CreateInfo {
        vec2u size;
    };

    static std::shared_ptr<SampleBuffer> create(const CreateInfo &createInfo);

    void writeSamples(const vec2u &idx, vec4f values);
    void writeWavelengths(const vec2u &idx, vec4f wavelengths);

    vec2u size() const;

private:
    SampleBuffer() = delete;
    SampleBuffer(const vec2u &size);

    std::unique_ptr<vec4f[]> _values;
    std::unique_ptr<vec4f[]> _wavelengths;

    vec2u _size;
};
} // namespace cobalt::render

#endif // COBALT_RENDER_RENDERTARGET_H
