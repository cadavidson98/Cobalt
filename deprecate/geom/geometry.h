#ifndef GEOMETRY_H
#define GEOMETRY_H
#include "hit_info.h"
#include "ray.h"
#include "bounding_box.h"
#include "boundable.h"

namespace cblt
{
    class Geometry : public Boundable
    {
        public:
            virtual bool Intersect(const Ray &ray, HitInfo &collision_pt) = 0;
    };
}
#endif  // GEOMETRY_H