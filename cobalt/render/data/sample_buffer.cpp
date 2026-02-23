#include "sample_buffer.h"

namespace cobalt::render {

namespace {

bool checkCreateInfo(const SampleBuffer::CreateInfo &createInfo) {
    if (createInfo.size.x == 0 || createInfo.size.y == 0) {
        return false;
    }

    return true;
}

} // namespace

std::shared_ptr<SampleBuffer> SampleBuffer::create(const SampleBuffer::CreateInfo &createInfo) {
    if (!checkCreateInfo(createInfo)) {
        return nullptr;
    }

    std::shared_ptr<SampleBuffer> renderTarget = std::shared_ptr<SampleBuffer>(new SampleBuffer(createInfo.size));

    return renderTarget;
}

void SampleBuffer::writeSamples(const vec2u &bufferIdx, vec4f values) {
    size_t idx = bufferIdx.x + bufferIdx.y * _size.x;
    _values[idx] = values;
}

void SampleBuffer::writeWavelengths(const vec2u &bufferIdx, vec4f wavelengths) {
    size_t idx = bufferIdx.x + bufferIdx.y * _size.x;
    _wavelengths[idx] = wavelengths;
}

SampleBuffer::SampleBuffer(const vec2u &size): _size{size} {

    const size_t numSamples = size_t(size.x) * size_t(size.y);

    _values = std::make_unique<vec4f[]>(numSamples);
    _wavelengths = std::make_unique<vec4f[]>(numSamples);
}

vec2u SampleBuffer::size() const {
    return _size;
}

} // namespace cobalt::render
