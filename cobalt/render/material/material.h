#ifndef CBLT_RENDER_MATERIAL_H
#define CBLT_RENDER_MATERIAL_H

#include "Ptexture.h"
#include "surface_function.h"
#include "surface_params.h"

#include "geometry/interpolation.h"
#include "math/vec2.h"

#include <memory>
#include <variant>

namespace cblt::render {

class CoMaterial {
public:
    template<typename parameterType>
    using ParameterSlot = std::variant<Ptex::PtexTexture *, parameterType>;

    using CoMaterialParams = CoPrincipledParams<ParameterSlot<vec4f>, float>;

    CoMaterial(CoMaterialParams params);
    CoSurfaceParams surfaceParamsAtCoordinates(const vec2f uvCoords, uint32_t faceIdx) const;

    CoMaterial(CoMaterial &&other);
    CoMaterial &operator=(CoMaterial &&other);

    // void sampleMaterialAtCoordinate(const CoRay &ray?, vec2f localCoorindates, uint32_t faceIdx) const;
private:
    CoMaterial(CoMaterial &) = delete;
    CoMaterial &operator=(CoMaterial &) = delete;

    CoMaterialParams _params;
    Ptex::PtexFilter *_filter;
};

} // namespace cblt::render
#endif // CBLT_RENDER_MATERIAL_H
