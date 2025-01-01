#ifndef COBALT_RENDER_RENDERTARGET_H
#define COBALT_RENDER_RENDERTARGET_H

#include "math/vec2.h"
#include "math/vec4.h"

#include <cstdint>
#include <memory>

namespace cblt::render {

class CoRenderTarget {
    public:
        enum RenderTargetFormat {
            RenderTargetFormatRGBA32Float,
        };

        enum RenderTargetTiling {
            RenderTargetTilingLinear,  // Arranges bytes in row major order
            RenderTargetTilingOptimal, // Arranges bytes in implementation-defined n x n tiles
        };

        struct CreateInfo {
                vec2u size = {0, 0};
                RenderTargetFormat format = RenderTargetFormatRGBA32Float;
                RenderTargetTiling tiling = RenderTargetTilingLinear;
        };

        static std::shared_ptr<CoRenderTarget> create(const CreateInfo &createInfo);
        ~CoRenderTarget();

        void Write(const vec2u &idx, const vec4f &value);
        const vec4f &at(const vec2u idx) const;

        vec2u size() const;
        RenderTargetFormat format() const;
        RenderTargetTiling tiling() const;
        uint8_t *data() const;

    private:
        CoRenderTarget();
        bool Init(const CreateInfo &createInfo);

        static constexpr uint32_t kTileSize = 32;
        static constexpr uint32_t kPixelsPerTile = kTileSize * kTileSize;

        static bool _CheckCreateInfo(const CreateInfo &createInfo);
        static size_t _CalculateIndex(const vec2u &idx, const vec2u &size, RenderTargetTiling tiling);

        // TODO: need to template or use generic bytes (uchar)
        vec4f *renderTargetBytes = nullptr;
        RenderTargetFormat renderTargetFormat = RenderTargetFormatRGBA32Float;
        RenderTargetTiling renderTargetTiling = RenderTargetTilingLinear;
        vec2u renderTargetSize = {0, 0};

}; // class
   // CoRenderTarget
} // namespace
  // cblt::render

#endif // COBALT_RENDER_RENDERTARGET_H
