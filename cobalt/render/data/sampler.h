#ifndef CBLT_RENDER_SAMPLER_H
#define CBLT_RENDER_SAMPLER_H

#include "byte_texture.h"
#include "texture.h"

namespace cblt::render {

class CoTexture;

class CoSampler : public CoTextureVisitor {
public:
    enum class EdgeMode {
        kClamp,
    };

    struct CreateOptions {
        size_t faceIdx = 0; // for per-face texture (PTex) sampling
    };

    struct CreateInfo {
        EdgeMode edgeMode = EdgeMode::kClamp;
        CreateOptions options = {};
    };

    static std::shared_ptr<CoSampler> create(const CreateInfo &createInfo);

    CoColor sample(const CoTexture &texture, vec2f coordinates);

    void visitTexture(const CoByteTexture &texture) override;
    void visitPTexture(const CoPTexture &texture) override;

private:
    CoSampler() = delete;
    CoSampler(const CreateInfo &createInfo);

    size_t _faceIdx;
    EdgeMode _edgeMode;
};

} // namespace cblt::render

#endif // CBLT_RENDER_SAMPLER_H
