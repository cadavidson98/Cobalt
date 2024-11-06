#ifndef CBLT_GEOM_INTERSECTION_H
#define CBLT_GEOM_INTERSECTION_H

namespace cblt::geom {

struct CoRay;
struct CoAxisAlignedBoundingBox;
struct CoTriangle;
struct CoSphere;

bool raySphereIntersection(const CoRay &ray, const CoSphere &sphere, float &tMin, float &tMax);

} // namespace cblt::geom

#endif // CBLT_GEOM_INTERSECTION_H
