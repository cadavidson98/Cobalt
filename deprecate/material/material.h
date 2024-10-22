#ifndef CBLT_MATERIAL_H
#define CBLT_MATERIAL_H

#include "image_lib.h"
#include "mat/sampler.h"
#include "geom/ray.h"
#include "geom/hit_info.h"

#include <memory>

namespace cblt
{
    class HitInfo;

    /**
     * An interface for defining materials. This defines the essential functions for illumination and sampling
     * for both Ray Tracing and Path Tracing.
     */ 
    class Material {
    public:    

        // Path Tracing Functions
        /**
         * \brief Calculates the material color at the given location and viewing angle
         * \param incoming Vector which defines the incident light ray
         * \param collision Data struct containing the collision location and normal
         * \param outgoing Vector which defines the outgoing direction towards the camera eye
         */ 
        virtual Color Sample(const Vec3 &incoming, Vec3 &outgoing, float &pdf, const HitInfo &collisionPt, std::shared_ptr<Sampler> &BRDF_sampler) = 0;
        virtual Color BRDF(const Vec3 &incoming, const Vec3 &outgoing, const HitInfo &collision_pt, float &pdf) = 0;
        virtual Color Emittance() = 0;
        float IOR() const { return ior_; };
        
    protected:
        Image *albedo_map_ = nullptr;  //! Optional albedo texture for better rendering
        Image *normal_map_ = nullptr;  //! Optional normal texture for better surface lighting
        float ior_;  //! index of refraction for the material
    };
}
#endif  // CBLT_MATERIAL_H