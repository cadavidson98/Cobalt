#ifndef CBLT_SHADE_HELPERS
#define CBLT_SHADE_HELPERS

#include "math/math_helpers.h"
#include "math/constants.h"

#include "fresnel.h"

#include <algorithm>
#include <cmath>

namespace cblt
{

    inline float GTR1(float cos_theta, float alpha)
    {
        if (alpha >= 1.f)
        {
            return 1.f / cblt::PI_f;
        }
        float alpha_sqr = cblt::sqr(alpha);
        float a_sqr_minus_1 = alpha_sqr - 1.f;
        return a_sqr_minus_1 / ((cblt::PI_f * std::log(alpha_sqr)) * (1.f + a_sqr_minus_1 * cblt::sqr(cos_theta)));
    }
    
    inline float GGX(float cos_theta, float alpha)
    {
        float cos_sqr = cos_theta * cos_theta;
        float rough_sqr = alpha * alpha;
        float denom = cos_sqr * (rough_sqr - 1.f) + 1.f;
        return rough_sqr / (cblt::PI_f * denom * denom);
    }

    inline float GGX_aniso(float alpha_x, float alpha_y, float cos_phi, float sin_phi, float cos_theta)
    {
        float A = cblt::sqr(cos_phi / alpha_x) + cblt::sqr(sin_phi / alpha_y);
        float denom = cblt::PI_f * alpha_x * alpha_y * cblt::sqr(cblt::sqr(cos_theta) + A);
        return 1.f / denom;
    }

    inline float SmithGeomAniso(const Vec3 &omega_i, const Vec3 &omega_o, const Vec3 &norm, 
                                const Vec3 &X, const Vec3 &Y, const float alpha_x, const float alpha_y)
    {
        float n_dot_i = Dot(omega_i, norm);
        float n_dot_o = Dot(omega_o, norm);

        // masking
        float sin_cos_aniso = cblt::sqr(Dot(omega_i, X) * alpha_x) + cblt::sqr(Dot(omega_i, Y) * alpha_y);
        float tan_sqr = (1.f - cblt::sqr(n_dot_i)) / (cblt::sqr(n_dot_i));
        float lambda_i = std::sqrt(1.f + sin_cos_aniso * tan_sqr);

        // shadowing
        sin_cos_aniso = cblt::sqr(Dot(omega_o, X) * alpha_x) + cblt::sqr(Dot(omega_o, Y) * alpha_y);
        tan_sqr = (1.f - cblt::sqr(n_dot_o)) / (cblt::sqr(n_dot_o));
        float lambda_o = std::sqrt(1.f + sin_cos_aniso * tan_sqr);

        return 2.0f / (lambda_i + lambda_o);
    }

    inline float SmithPartialGeom(const Vec3 &omega, const Vec3 &halfway, float alpha)
    {
        float w_dot_h = AbsDot(omega, halfway);
    
        float tan_sqr = (1.f - w_dot_h * w_dot_h) / (w_dot_h * w_dot_h);
        return 2.f / (1.f + std::sqrt(1.f + alpha * alpha * tan_sqr));
    }

    inline float SmithPartialGeomAniso(float alpha_x, float alpha_y, float n_dot_v, float v_dot_x, float v_dot_y)
    {
        float sin_cos_aniso = cblt::sqr(v_dot_x * alpha_x) + cblt::sqr(v_dot_y * alpha_y);
        float tan_sqr = (1.f - n_dot_v * n_dot_v) / (n_dot_v * n_dot_v);
        float denom = 1.f + std::sqrt(1.f + sin_cos_aniso * tan_sqr);
        return 2.f / denom;
    }

    inline void OrthonormalBasis(const Vec3 &normal, Vec3 &tangent, Vec3 &bitangent)
    {
        float sign = std::copysign(1.f, normal.z);
        float a = -1.f / (sign + normal.z);
        float b = normal.x * normal.y * a;
        tangent = Vec3(1.f + sign * normal.x * normal.x * a, sign * b, -sign * normal.x);
        bitangent = Vec3(b, sign + normal.y * normal.y * a, -normal.y);
    }
}

#endif  // CBLT_SHADE_HELPERS