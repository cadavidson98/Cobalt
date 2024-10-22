#include "microfacet_specular.h"

#include "fresnel.h"
#include "ggx_function.h"
#include "math/math_utils.h"
#include "math/vec3.h"
#include "smith_geometry_function.h"
#include "surface_utils.h"

namespace cblt::material {

CoColor CoMicrofacetSpecular::BSDF(
    const vec3f &omegaI,
    const vec3f &omegaO,
    const vec3f &normal,
    const CoColor &incidentRadiance,
    const CoSurfaceParams &surfaceParams
) const {
    const float alpha = sqr(surfaceParams.roughness);

    const vec3f halfway = normalize(omegaI + omegaO);

    float alphaX, alphaY;
    calculateAnisotropyWeights(alpha, surfaceParams.anisotropic, alphaX, alphaY);

    // Approximate microfacets using the GGX normal distribution function
    const float nDotH = dot(normal, halfway);
    const float oDotH = dot(omegaO, halfway);
    const float chiHN = static_cast<float>(nDotH > 0.f);
    // TODO: pass orthonormal basis, or compute shading in orthonormal basis
    // (mind the jacobian :))
    const float cosPhi = dot(halfway, X);
    const float sinPhi = dot(halfway, Y);
    const float cosTheta = dot(halfway, Z);
    const float D = GGX_aniso(alphaX, alphaY, cosPhi, sinPhi, cosTheta);
    // Geometric attenuation - Smith Partial Geometry
    const float inDotH = dot(omegaI, halfway);
    const float G = smithGeomAniso(omegaI, omegaO, normal, X, Y, alphaX, alphaY);
    // Approximate geometry of microfacets using Schlick
    const float F0 = sqr((1.f - surfaceParams.ior) / (1.f + surfaceParams.ior));
    const float F = FresnelSchlick(F0, oDotH);
    // TODO: can't reflect alpha...
    return incidentRadiance * surfaceParams.specular * (chiHN * D * F * G) /
           (4.f * absDot(omegaI, normal) * absDot(omegaO, normal));
}

} // namespace cblt::material
