#include "hit_info.h"
#include "math/constants.h"
#include "mat/image_lib.h"

#include "scene_prim.h"

namespace cblt
{
    HitInfo::HitInfo()
    {
        float hit_time = inf_F;
        shading_basis = Mat4(1.f);
        medium_ior  = 1.f;
        m = nullptr;
        geom = nullptr;
    }

    Color HitInfo::Emittance()
    {
        return Color::GreyScale(0.f);
    }
}