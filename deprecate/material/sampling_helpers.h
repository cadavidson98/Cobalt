#ifndef SAMPLE_HELPERS
#define SAMPLE_HEADERS

#include "math/math_helpers.h"
#include "math/constants.h"
#include "shading_helpers.h"
#include "mat4.h"
#include "sampler.h"

#include <memory>

namespace cblt
{
    inline Vec2 ConcentricPointOnUnitDisk(const float u1, const float u2)
    {
        // uniform sample disk, then project onto the hemisphere using malley's method
        // concentric mapping, courtesy of PBR
        // get values in [-1, 1]
        float norm_u1 = 2.f * u1 - 1.f, norm_u2 = 2.f * u2 - 1.f;
        float radius(0.f), theta(0.f);

        if (norm_u1 != 0.f || norm_u2 != 0.f)
        {
            if (std::abs(norm_u1) > std::abs(norm_u2))
            {
                radius = norm_u1;
                theta = .25f * PI_f * (norm_u2 / norm_u1);
            }
            else
            {
                radius = norm_u2;
                theta = .5f * PI_f - .25f * PI_f * (norm_u1 / norm_u2);
            }
        }

        return Vec2(radius * std::cos(theta), radius * std::sin(theta));
    }

    inline Vec3 RandomUnitVectorInHemisphere(const Vec3 &bitangent, const Vec3 &normal, const Vec3 &tangent, 
                                             float &pdf, const float u1, const float u2)
    {
        float radius = std::sqrtf(std::max(0.f, 1.f - cblt::sqr(u1)));
        float theta = 2.f * cblt::PI_f * u2;

        float x = radius * std::cos(theta);
        float z = radius * std::sin(theta);
        
        Vec3 rand_vec(x, u1, z);
        
        Mat4 BNT(Vec4(bitangent, 0.f), Vec4(normal, 0.f), Vec4(tangent, 0.f), Vec4(0.f, 0.f, 0.f, 1.f));
        Vec4 out_vec = BNT * Vec4(rand_vec, 0);

        Vec3 result = Normalize(Vec3(out_vec.x, out_vec.y, out_vec.z));
        if(Dot(result, normal) < 0) {
            // make sure the sample is inside the hemisphere
            result = result * -1.f;
        }
        pdf = .5f * INV_PI_f;
        return result;
    }

    inline Vec3 RandomUnitVectorInCosineWeightedHemisphere(const Vec3 &bitangent, const Vec3 &normal, const Vec3 &tangent,
                                                           float &pdf, const float u1, const float u2)
    {
        Vec2 sample = ConcentricPointOnUnitDisk(u1, u2);
        float y = std::sqrt(std::max(0.f, 1.f - sqr(sample.x) - sqr(sample.y)));

        Vec3 rand_vec(sample.x, y, sample.y);
        
        Mat4 BNT(Vec4(bitangent, 0.f), Vec4(normal, 0.f), Vec4(tangent, 0.f), Vec4(0.f, 0.f, 0.f, 1.f));
        Vec4 out_vec = BNT * Vec4(rand_vec, 0);

        Vec3 result = Normalize(Vec3(out_vec.x, out_vec.y, out_vec.z));
        pdf = Dot(result, normal) * INV_PI_f;
        return result;
    }

    inline Vec3 RandomUnitVectorInGTR1(const Vec3 &bitangent, const Vec3 &normal, const Vec3 &tangent, const Vec3 &outgoing,
                                       const float alpha, float &pdf, const float u1, const float u2)
    {
        Mat4 Tan_To_World(Vec4(bitangent, 0.f), Vec4(normal, 0.f), Vec4(tangent, 0.f), Vec4(0.f, 0.f, 0.f, 1.f));
        Mat4 World_To_Tan = OrthoInverse(Tan_To_World);
        Vec4 out_tan = World_To_Tan * Vec4(outgoing, 0.f);
        Vec3 w_o = { out_tan.x, out_tan.y, out_tan.z };

        float alpha_sqr = sqr(alpha);
        float cos_theta = std::sqrt((1.f - std::pow(alpha_sqr, 1.f - u1)) / (1.f - alpha_sqr));
        float sin_theta = std::sqrt(1.f - sqr(cos_theta));

        float phi = 2.f * cblt::PI_f * u2;
        Vec3 sample = { sin_theta * std::cos(phi),
                        cos_theta,
                        sin_theta * std::sin(phi) };

        sample = Normalize(sample);

        float O_dot_S = Dot(w_o, sample);
        float O_dot_N = Dot(w_o, Y_axis_F);
        float S_dot_N = Dot(sample, Y_axis_F);
        
        if (O_dot_N * S_dot_N < 0.f) {
            sample = -sample;
            O_dot_S = -O_dot_S;
        }

        Vec3 w_i = Normalize(2.f * O_dot_S * sample - w_o);

        // calculate the pdf for the reflection vector in cartesian coords
        pdf = GTR1(cos_theta, alpha) * cos_theta / (4.f * O_dot_S);
    
        // take back from tangent space to world space
        Vec4 out_vec = Tan_To_World * Vec4(w_i, 0.f);
        Vec3 result = { out_vec.x, out_vec.y, out_vec.z };
        return Normalize(result);
    }

    // Adapted from https://schuttejoe.github.io/post/ggximportancesamplingpart1/
    inline Vec3 RandomUnitVectorInGGX(const Vec3 &bitangent, const Vec3 &normal, const Vec3 &tangent, const Vec3 &outgoing, 
                                      const float alpha, float &pdf, const float u1, const float u2) {
        Mat4 Tan_To_World(Vec4(bitangent, 0.f), Vec4(normal, 0.f), Vec4(tangent, 0.f), Vec4(0.f, 0.f, 0.f, 1.f));
        Mat4 World_To_Tan = OrthoInverse(Tan_To_World);
        Vec4 out_tan = World_To_Tan * Vec4(outgoing, 0.f);
        Vec3 w_o = { out_tan.x, out_tan.y, out_tan.z };

        // Formula from: https://agraphicsguy.wordpress.com/2015/11/01/sampling-microfacet-brdf/
        float alpha_sqr = alpha * alpha;
        float theta = std::acos((1.f - u1) / (u1 * (alpha_sqr - 1.f) + 1.f));
        float phi = 2.f * cblt::PI_f * u2;

        float cos_theta = std::cos(theta);
        float sin_theta = std::sin(theta);

        // convert from polar to cartesian
        Vec3 sample = { sin_theta * std::cos(phi),
                        cos_theta,
                        sin_theta * std::sin(phi) };
        sample = Normalize(sample);
        // we sampled a normal, but we want a reflection vector, so reflect
        float O_dot_S = Dot(w_o, sample);
        float O_dot_N = Dot(w_o, Y_axis_F);
        float S_dot_N = Dot(sample, Y_axis_F);
        if (O_dot_N * S_dot_N < 0.f) {
            sample = -sample;
            O_dot_S = -O_dot_S;
        }

        Vec3 w_i = Normalize(2.f * O_dot_S * sample - w_o);
        
        // calculate the pdf for the reflection vector in cartesian coords
        pdf = GGX(cos_theta, alpha) * cos_theta / (4.f * O_dot_S);
    
        // take back from tangent space to world space
        Vec4 out_vec = Tan_To_World * Vec4(w_i, 0.f);
        Vec3 result = { out_vec.x, out_vec.y, out_vec.z };
        return Normalize(result);
}

    inline Vec3 RandomUnitVectorInGGXAniso(const Vec3 &bitangent, const Vec3 &normal, const Vec3 &tangent, const Vec3 &outgoing, 
                                      const float alpha_x, const float alpha_y, float &pdf, const float u1, const float u2) {
        Mat4 Tan_To_World(Vec4(bitangent, 0.f), Vec4(normal, 0.f), Vec4(tangent, 0.f), Vec4(0.f, 0.f, 0.f, 1.f));
        Mat4 World_To_Tan = OrthoInverse(Tan_To_World);
        Vec4 out_tan = World_To_Tan * Vec4(outgoing, 0.f);
        Vec3 w_o = { out_tan.x, out_tan.y, out_tan.z };

        // Formula from: https://agraphicsguy.wordpress.com/2018/07/18/sampling-anisotropic-microfacet-brdf/
        float phi = std::atan((alpha_y / alpha_x) * std::tan(2 * cblt::PI_f * u1 + .5f * cblt::PI_f));
        phi += cblt::PI_f * (u1 > .5f);
        
        float alpha_sqr = cblt::sqr(std::cos(phi) / alpha_x) + cblt::sqr(std::sin(phi) / alpha_y);
        float tan_sqr = u2 / ((1.f - u2) * alpha_sqr);

        float theta = std::atan(std::sqrt(tan_sqr));

        float cos_theta = std::cos(theta);
        float sin_theta = std::sin(theta);
        float cos_phi = std::cos(phi);
        float sin_phi = std::sin(phi);
        // convert from polar to cartesian
        Vec3 sample = { sin_theta * cos_phi,
                        cos_theta,
                        sin_theta * sin_phi };
        sample = Normalize(sample);
        // we sampled a normal, but we want a reflection vector, so reflect
        float O_dot_S = Dot(w_o, sample);
        float O_dot_N = w_o.y;
        float S_dot_N = sample.y;
        if (O_dot_N * S_dot_N < 0.f) {
            sample = -sample;
            O_dot_S = -O_dot_S;
        }

        Vec3 w_i = Normalize(2.f * O_dot_S * sample - w_o);
        
        // calculate the pdf for the reflection vector in cartesian coords
        pdf = GGX_aniso(alpha_x, alpha_y, sample.x, sample.z, sample.y) * cos_theta / (4.f * O_dot_S);
    
        // take back from tangent space to world space
        Vec4 out_vec = Tan_To_World * Vec4(w_i, 0.f);
        Vec3 result = { out_vec.x, out_vec.y, out_vec.z };
        return Normalize(result);
    }

    // A Simpler and Exact Sampling Routine for the GGX Distribution of Visible Normals by Eric Heitz
    // https://hal.archives-ouvertes.fr/hal-01509746/document
    inline Vec3 SampleVisibleGGX(const Vec3 &outgoing, const float a_x, const float a_y, const float u1, const float u2)
    {
        // Stretch Halfway Vector
        Vec3 v = Normalize(Vec3(a_x * outgoing.x, outgoing.y, a_y * outgoing.z));

        // Construct Orthonormal Basis: TODO- try replacing with RMth::OrthonormalBasis()
        Vec3 bitangent = (v.y < 0.999f) ? Normalize(Cross(v, Y_axis_F)) : X_axis_F;
        Vec3 tangent = Cross(bitangent, v);

        // sample uniform hemisphere using polar coords
        float r = std::sqrt(u1);
        
        float a = 1.f / (1.f + v.y);
        float phi = (u2 < a) ? (u2 / a) * cblt::PI_f : cblt::PI_f + (u2 - a) / (1.f - a) * cblt::PI_f;
        float t1 = r * std::cos(phi);
        float t2 = r * std::sin(phi) * ((u2 < a) ? 1.f : v.y);
        
        Vec3 normal = bitangent * t1 + tangent * t2 + v * std::sqrt(std::max(0.f, 1.f - cblt::sqr(t1) - cblt::sqr(t2)));
        // unstretch
        Vec3 normal_e = Normalize(Vec3(a_x * normal.x, std::max(0.f, normal.y), a_y * normal.z));
        return normal_e;
    }
}

#endif