#ifndef CBLT_FRESNEL_H
#define CBLT_FRESNEL_H

#include "math/math_helpers.h"
#include <cmath>

namespace cblt
{
    inline float FresnelSchlick(float F0, float cos_theta) 
    {
        return F0 + (1.f - F0) * std::pow(1.f - cos_theta, 5);
    }

    inline float FresnelCoef(float cos_theta)
    {
        float f = 1.f - cos_theta;
        return f * sqr(sqr(f));
    }

    inline float DielectricFresnel(float cos_theta_i, float eta_i, float eta_t)
    {
        if (cos_theta_i < 0.f)
        {
            std::swap(eta_i, eta_t);
            cos_theta_i *= -1.f;
        }

        // compute cos_theta_t using Snell's law
        float sin_theta_i = std::sqrt(1.f - sqr(cos_theta_i));
        float sin_theta_t = sin_theta_i * (eta_i / eta_t);
        float cos_theta_t = std::sqrt(1.f - sqr(sin_theta_t));

        if (sin_theta_t >= 1.f)
        {
            // total internal reflection
            return 1.f;
        }
        // compute reflectance for parallel polarized light:
        float f_par = (eta_t * cos_theta_i - eta_i * cos_theta_t) / (eta_t * cos_theta_i + eta_i * cos_theta_t);
        // compute reflectance for perpendicular polarized light:
        float f_per = (eta_i * cos_theta_i - eta_t * cos_theta_t) / (eta_i * cos_theta_i + eta_t * cos_theta_t);
        return .5f * (sqr(f_par) + sqr(f_per));
    }

    inline float SchlickR0(float eta_i, float eta_t)
    {
        return sqr((eta_i - eta_t) / (eta_i + eta_t));
    }
}

#endif  // CBLT_FRESNEL_H