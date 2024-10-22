#ifndef CBLT_MIS_H
#define CBLT_MIS_H

#include <cmath>

namespace cblt
{
    inline float PowerHeuristic(float alpha, float beta, float scale)
    {
        float a_scale = std::pow(alpha, scale), b_scale = std::pow(beta, scale);
        return a_scale / (a_scale + b_scale);
    }
}

#endif  // CBLT_MIS_H