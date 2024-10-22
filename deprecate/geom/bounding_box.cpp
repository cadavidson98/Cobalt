#include "bounding_box.h"

namespace cblt {
    
    BoundingBox::BoundingBox(const Vec3 &mn, const Vec3 &mx) :
        min_(mn), max_(mx)
    {
        CalculateCenter();
    }

    void BoundingBox::CalculateCenter() {
        cen_ = (max_ + min_) * .5f;
    }

    float BoundingBox::SurfaceArea() const {
        return  2.f * (
                ((max_.x - min_.x) * (max_.y - min_.y)) +
                ((max_.x - min_.x) * (max_.z - min_.z)) +
                ((max_.z - min_.z) * (max_.y - min_.y))
                );
    }

    BoundingBox BoundingBox::Union(const BoundingBox &other) const {
        BoundingBox uni;
        uni.min_.x = std::min(min_.x, other.min_.x);
        uni.min_.y = std::min(min_.y, other.min_.y);
        uni.min_.z = std::min(min_.z, other.min_.z);

        uni.max_.x = std::max(max_.x, other.max_.x);
        uni.max_.y = std::max(max_.y, other.max_.y);
        uni.max_.z = std::max(max_.z, other.max_.z);

        return uni;
    }

    bool BoundingBox::Intersect(const Ray &ray, float &intersect_time)
    {
        float tmin(-inf_F), tmax(inf_F);
        float t1[3], t2[3];
        // looping so compiler can more explicitly unroll & pack into SIMD registers
        for (int i = 0; i < 3; ++i)
        {
            t1[i] = (min_.xyz[i] - ray.pos.xyz[i]) * ray.inv.xyz[i];
            t2[i] = (max_.xyz[i] - ray.pos.xyz[i]) * ray.inv.xyz[i];
        }

        tmin = std::min(t1[0], t2[0]);
        tmax = std::max(t1[0], t2[0]);
      
	    tmin = std::max(tmin, std::min(t1[1], t2[1]));
        tmax = std::min(tmax, std::max(t1[1], t2[1]));
    	    
	    tmin = std::max(tmin, std::min(t1[2], t2[2]));
        tmax = std::min(tmax, std::max(t1[2], t2[2]));
        
        if (tmax > 0.f && tmax >= tmin)
        {
            float use_min = static_cast<float>(tmin >= 0.f);
            intersect_time = use_min * tmin + !use_min * tmax;
            return true;
        }
        return false;
    }
}