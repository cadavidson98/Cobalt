#include "disney_principled.h"
#include "shading_helpers.h"
#include "math_helpers.h"
#include "sampling_helpers.h"

namespace cblt
{

    DisneyPrincipledMaterial::DisneyPrincipledMaterial(Color & base_color, float subsurface, float metallic, float specular, 
            float specular_tint, float roughness, float anisotropic, float sheen, float sheen_tint,
            float clearcoat, float clearcoat_gloss, float ior, bool thin) :
            base_(base_color), subsrfc_(subsurface), metal_(metallic), spec_(specular),
            spec_tint_(specular_tint), rough_(roughness), aniso_(anisotropic), sheen_(sheen),
            sheen_tint_(sheen_tint), clrcoat_(clearcoat), clrcoat_gloss_(clearcoat_gloss), thin_(thin)
    {
        ior_ = ior;
        
        float diffuse_weight = 1.f - metal_;
        float specular_weight = metal_;
        float clrcoat_weight = .25f * clrcoat_;
        float tot = diffuse_weight + specular_weight + clrcoat_weight;
        d_pdf_ = diffuse_weight;
        s_pdf_ = specular_weight;
        cc_pdf_ = clrcoat_weight;
    }

    Color DisneyPrincipledMaterial::Sample(const Vec3 &outgoing, Vec3 &incoming, float &pdf, const HitInfo &collision_pt, std::shared_ptr<Sampler> &BRDF_sampler)
    {
        // First, determine which BRDF we need to use for this ray
        Vec3 bitangent, tangent;
        OrthonormalBasis(collision_pt.norm, bitangent, tangent);

        // choose lobe to sample
        float lobe_weight;
        BRDF_sampler->Next1D(lobe_weight);
        // lobe weights

        if (lobe_weight < d_pdf_)
        {
            // diffuse lobe sampling
            float u1, u2;
            BRDF_sampler->Next2D(u1, u2);
            incoming = cblt::RandomUnitVectorInCosineWeightedHemisphere(bitangent, collision_pt.norm, tangent, pdf, u1, u2);
        }
        else if (lobe_weight < d_pdf_ + s_pdf_)
        {
            float alpha_x, alpha_y;
            GetAnisoParams(alpha_x, alpha_y);

            float u1, u2;
            BRDF_sampler->Next2D(u1, u2);

            Mat4 Tan_To_World(Vec4(bitangent, 0.f), Vec4(collision_pt.norm, 0.f), Vec4(tangent, 0.f), Vec4(0.f, 0.f, 0.f, 1.f));
            Mat4 World_To_Tan = OrthoInverse(Tan_To_World);
            Vec4 out_tan = World_To_Tan * Vec4(outgoing, 0.f);
            Vec3 w_o(out_tan.x, out_tan.y, out_tan.z);

            Vec3 sample_h = SampleVisibleGGX(w_o, alpha_x, alpha_y, u1, u2);
            // reflect about sampled halfway to get the incoming light vector
            float O_dot_S = Dot(w_o, sample_h);
            float O_dot_N = w_o.y;
            float S_dot_N = sample_h.y;

            Vec3 w_i = Normalize(2.f * O_dot_S * sample_h - w_o);
            float D_aniso = GGX_aniso(alpha_x, alpha_y, sample_h.x, sample_h.z, sample_h.y);
            float G1_aniso = SmithPartialGeomAniso(alpha_x, alpha_y, std::max(0.f, O_dot_N), w_o.x, w_o.z);
            pdf = (D_aniso * G1_aniso) / (4.f * O_dot_N);

            // take back from tangent space to world space
            Vec4 out_vec = Tan_To_World * Vec4(w_i, 0.f);
            incoming = Normalize(Vec3(out_vec.x, out_vec.y, out_vec.z));
        }
        else
        {
            float u1, u2;
            BRDF_sampler->Next2D(u1, u2);

            incoming = RandomUnitVectorInGTR1(bitangent, collision_pt.norm, tangent, outgoing, cblt::lerp<float>(.1f, .001f, clrcoat_gloss_), pdf, u1, u2);
        }
        return BRDF(incoming, outgoing, collision_pt, pdf);
    }

    // nomenclature guide:
    // incoming -> omega_i -> toLight (L) -> The light arriving at this point
    // outgoing -> omega_o -> toEye (V) -> The light reflected at this point
    Color DisneyPrincipledMaterial::BRDF(const Vec3 &incoming, const Vec3 &outgoing, const HitInfo &collision_pt, float &pdf)
    {
        float d_pdf, s_pdf, c_pdf;
        if (Dot(incoming, collision_pt.norm) < eps_zero_F || Dot(outgoing, collision_pt.norm) < eps_zero_F)
        {
            // no support for transmission (yet), so any event below the hemisphere should be non-existant
            pdf = 0.f;
            return Color::GreyScale(0.f);
        }

        Vec3 halfway = Normalize(incoming + outgoing);
        Color diffuse = DisneyDiffuse(incoming, outgoing, halfway, collision_pt, d_pdf);
        Color specular = DisneySpecular(incoming, outgoing, halfway, collision_pt, s_pdf);
        Color clearcoat = DisneyClearcoat(incoming, outgoing, halfway, collision_pt, c_pdf);
        Color sheen = DisneySheen(incoming, outgoing, halfway, collision_pt);
        pdf = (d_pdf_ * d_pdf + s_pdf_ * s_pdf + cc_pdf_ * c_pdf);
        
        return (diffuse + sheen) * (1.f - metal_) + specular + clearcoat;
    }

    Color DisneyPrincipledMaterial::DisneyDiffuse(const Vec3 &incoming, const Vec3 &outgoing, const Vec3 &halfway, const HitInfo &collision_pt, float &pdf)
    {
        float n_dot_i = Dot(collision_pt.norm, incoming);
        float n_dot_o = Dot(collision_pt.norm, outgoing);

        float h_dot_i = Dot(halfway, incoming);
        float fresnel_i = FresnelSchlick(0.f, n_dot_i);
        float fresnel_o = FresnelSchlick(0.f, n_dot_o);

        float fresnel_d90 = 2.f * rough_ * cblt::sqr(h_dot_i);
        float diffuse = (1.f - .5f * fresnel_i) * (1.f - .5f * fresnel_o);
        
        // retro-reflection
        float retro_refl = fresnel_d90 * (fresnel_i + fresnel_o + fresnel_i * fresnel_o * (fresnel_d90 - 1.f));
        // TODO: Proper Subsurface
        /*float fresnel_ss90 = rough_ * cblt::sqr(h_dot_i);
        float subsurface = 1.25f * 
            (cblt::lerp(1.f, fresnel_ss90, n_dot_i) * cblt::lerp(1.f, fresnel_ss90, n_dot_o) *
            (1.f / (std::abs(n_dot_i)) + std::abs(n_dot_o) - 0.5f) + .5f);*/
        
        pdf = n_dot_i * INV_PI_f;

        return base_ * INV_PI_f * (diffuse + retro_refl);
    }

    Color DisneyPrincipledMaterial::DisneySpecular(const Vec3 &incoming, const Vec3 &outgoing, const Vec3 &halfway, const HitInfo &collision_pt, float &pdf)
    {
        Vec3 X, Y;
        OrthonormalBasis(collision_pt.norm, X, Y);

        Color white = Color::GreyScale(1.f);

        float n_dot_i = Dot(collision_pt.norm, incoming);
        float n_dot_o = Dot(collision_pt.norm, outgoing);
        float n_dot_h = Dot(collision_pt.norm, halfway);

        if (n_dot_i <= 0.f || n_dot_o <= 0.f)
        {
            pdf = 0.f;
            return Color::GreyScale(0.f);
        }

        float h_dot_i = Dot(halfway, incoming);
        float h_dot_x = Dot(halfway, X);
        float h_dot_y = Dot(halfway, Y);

        float fresnel_h = FresnelCoef(h_dot_i);

        float luminance = base_.Luminance();
        Color tint = (luminance > 0.f) ? base_ / luminance : white;
        Color k_s = cblt::lerp<Color>(white, tint, spec_tint_) * 0.08f * spec_; 
        Color spec = cblt::lerp<Color>(k_s, base_, metal_);

        float alpha_x, alpha_y;
        GetAnisoParams(alpha_x, alpha_y);

        float D_s = GGX_aniso(alpha_x, alpha_y, h_dot_x, h_dot_y, n_dot_h);
        float G_s = SmithGeomAniso(incoming, outgoing, collision_pt.norm, X, Y, alpha_x, alpha_y);
        Color F_s = spec + (white - spec) * fresnel_h;

        float G1_aniso = SmithPartialGeomAniso(alpha_x, alpha_y, std::max(0.f, n_dot_o), Dot(outgoing, X), Dot(outgoing, Y));
        pdf = D_s * G1_aniso / (4.f * n_dot_o);

        return F_s * D_s * G_s / (4.f * n_dot_i * n_dot_o);
    }

    Color DisneyPrincipledMaterial::DisneyClearcoat(const Vec3 &incoming, const Vec3 &outgoing, const Vec3 &halfway, const HitInfo &collision_pt, float &pdf)
    {
        float n_dot_o = AbsDot(outgoing, collision_pt.norm);
        float n_dot_i = AbsDot(incoming, collision_pt.norm);
        float n_dot_h = AbsDot(halfway, collision_pt.norm);

        float h_dot_i = Dot(halfway, incoming);

        float D_cc = GTR1(n_dot_h, cblt::lerp<float>(.1f, .001f, clrcoat_gloss_));
        float G_cc = SmithPartialGeom(incoming, collision_pt.norm, .25f) * SmithPartialGeom(outgoing, collision_pt.norm, .25f);
        // IOR 1.5 -> Fresnel .04
        float F_cc = FresnelSchlick(.04f, h_dot_i);

        pdf = D_cc * n_dot_h / (4.f * h_dot_i);

        return Color::GreyScale(.25f * clrcoat_ * D_cc * F_cc * G_cc);
    }

    Color DisneyPrincipledMaterial::DisneySheen(const Vec3 &incoming, const Vec3 &outgoing, const Vec3 &halfway, const HitInfo &collision_pt)
    {
        Color tint = base_.Luminance() > 0.f ? base_ / base_.Luminance() : Color::GreyScale(1.f);
        return lerp(Color::GreyScale(1.f), tint, sheen_tint_) * sheen_ * FresnelCoef(Dot(halfway, incoming));
    }

    Color DisneyPrincipledMaterial::DisneyTransmission(const Vec3 &incoming, const Vec3 &outgoing, const Vec3 &halfway, const HitInfo &collision_pt)
    {
        // TODO
        return Color::GreyScale(0.f);
    }

    Color DisneyPrincipledMaterial::Emittance()
    {
        return Color(0.f, 0.f, 0.f);
    }

    void DisneyPrincipledMaterial::GetAnisoParams(float &a_x, float &a_y)
    {
        float aspect = std::sqrt(1.f - 0.9f * aniso_);
        a_x = std::max(.0001f, cblt::sqr(rough_) / aspect);
        a_y = std::max(.0001f, cblt::sqr(rough_) * aspect);
    }
}