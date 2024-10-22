#ifndef CBLT_CAMERA_H
#define CBLT_CAMERA_H

#include "math/vec.h"
#include "geom/ray.h"
#include "math/constants.h"
namespace cblt
{

    enum camera_type
    {
        orthographic,
        perspective,
        cabinet,
        cavalier
    };

    class Camera
    {
    public:
        Camera(Vec3 eye = Vec3(0.f,0.f,0.f), Vec3 forward = Vec3(0.f,0.f,-1.f), Vec3 up = Vec3(0.f,1.f,0.f), float horiz_half_fov = PI_f/4.f, camera_type proj_type = camera_type::perspective);
        void ConfigureExtent(float img_w, float img_h);
        Vec3 Eye() { return cam_eye_; };
        Ray CreateRay(float img_x, float img_y);
    private:
        camera_type cam_type_;
        Vec3 cam_eye_;
        Vec3 cam_up_;
        Vec3 cam_fwd_;
        Vec3 cam_right_;
        float half_angle_fov_;  //! half the horizontal field of view (FOV)
        float near_plane_;  //! distance from eye to the image plane
    };
}
#endif  // CBLT_CAMERA_H