#ifndef CBLT_RAY_H
#define CBLT_RAY_H

#include "math/vec.h"
#include "math/constants.h"
namespace cblt
{
    struct Ray
    {
        Vec3 pos;
        Vec3 dir;
        Vec3 inv;
        Ray() {};
        Ray(Vec3 _pos, Vec3 _dir)
        {
            pos = _pos;
            dir = _dir;
            inv = Vec3(1.0f / (dir.x + eps_zero_F), 1.0f / (dir.y + eps_zero_F), 1.0f / (dir.z + eps_zero_F));
        }
    };
}

#endif  // CBLT_RAY_H