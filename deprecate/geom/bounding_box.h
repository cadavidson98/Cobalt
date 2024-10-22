#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include "math\vec.h"
#include "math\constants.h"
#include "ray.h"
#include <numeric>

namespace cblt {
    
    struct BoundingBox {
        Vec3 min_ = { inf_F, inf_F, inf_F };
        Vec3 max_ = { minus_inf_F, minus_inf_F, minus_inf_F };
        Vec3 cen_ = { 0.f, 0.f, 0.f};

        BoundingBox() {};
        BoundingBox(const Vec3 &mn, const Vec3 &mx);
        void CalculateCenter();
        BoundingBox Union(const BoundingBox &other) const;
        float SurfaceArea() const;
        bool Intersect(const Ray &ray, float &intersect_time);
    };

}
#endif  // BOUNDING_BOX_H