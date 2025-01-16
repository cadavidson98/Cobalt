#ifndef CBLT_RENDER_MATERIAL_H
#define CBLT_RENDER_MATERIAL_H

#include "interpolation.h"
#include "surface_function.h"
#include "surface_params.h"

#include "vec2.h"

#include "Ptexture.h"

#include <memory>

namespace cblt::render {

class CoMaterial {
    public:
    CoMaterial(CoSurfaceParams params, Ptex::PtexTexture *texture);
    CoSurfaceParams surfaceParamsAtCoordinates(const vec2f uvCoords, uint32_t faceIdx) const;
    // void sampleMaterialAtCoordinate(const CoRay &ray?, vec2f localCoorindates, uint32_t faceIdx) const;
    private:
    CoSurfaceParams _params;
    Ptex::PtexTexture *_texture;
    Ptex::PtexFilter *_filter;
};

} // namespace cblt::render
#endif // CBLT_RENDER_MATERIAL_H
