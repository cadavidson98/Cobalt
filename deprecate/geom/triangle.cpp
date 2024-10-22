#include "triangle.h"


#include <algorithm>

namespace cblt {

	Triangle::Triangle() : use_vertex_norms_(false), use_vertex_uvs_(false), face_norm_len_(0.f)
	{
		
	};

    Triangle::Triangle(const Vec3 &p1, const Vec3 &p2, const Vec3 &p3, std::shared_ptr<Material> &mat) :
    pos1_(p1), pos2_(p2), pos3_(p3)
    {
        use_vertex_norms_ = use_vertex_uvs_ = false;
        // calculate the per-face normal
        Vec3 v1 = pos3_ - pos1_;
        Vec3 v2 = pos2_ - pos1_;
        face_norm_ = Cross(v1, v2);
        face_norm_len_ = Magnitude(face_norm_);
        face_norm_ = Normalize(face_norm_);
		mat_ = mat; 
    }

    Triangle::Triangle(const Vec3 &p1, const Vec3 &p2, const Vec3 &p3, 
                       const Vec3 &n1, const Vec3 &n2, const Vec3 &n3, std::shared_ptr<Material> &mat) :
    pos1_(p1), pos2_(p2), pos3_(p3), norm1_(n1), norm2_(n2), norm3_(n3)
    {
        use_vertex_norms_ = true;
        use_vertex_uvs_ = false;
		// calculate the per-face normal
        Vec3 v1 = pos3_ - pos1_;
        Vec3 v2 = pos2_ - pos1_;
        face_norm_ = Cross(v1, v2);
        face_norm_len_ = Magnitude(face_norm_);
        face_norm_ = Normalize(face_norm_);
		mat_ = mat;
    }

    Triangle::Triangle(const Vec3 &p1, const Vec3 &p2, const Vec3 &p3, 
                      const Vec3 &n1, const Vec3 &n2, const Vec3 &n3,
                      const Vec2 &uv1, const Vec2 &uv2, const Vec2 &uv3, std::shared_ptr<Material> &mat) :
    pos1_(p1), pos2_(p2), pos3_(p3), 
    norm1_(n1), norm2_(n2), norm3_(n3),
    uv1_(uv1), uv2_(uv2), uv3_(uv3)
    {
        use_vertex_norms_ = use_vertex_uvs_ = true;
		// calculate the per-face normal
        Vec3 v1 = pos3_ - pos1_;
        Vec3 v2 = pos2_ - pos1_;
        face_norm_ = Cross(v1, v2);
        face_norm_len_ = Magnitude(face_norm_);
        face_norm_ = Normalize(face_norm_);
		mat_ = mat;
    }

    bool Triangle::Intersect(const Ray &ray, HitInfo &collision_pt)
    {

  	    float dt = Dot(ray.dir, face_norm_);
  	    if (std::abs(dt) < eps_zero_F)
        {
  	        // the ray is parallel, so return false
  	        return false;
  	    }

  	    float hit_t = Dot(pos1_ - ray.pos, face_norm_) / dt;
  	    if (hit_t < eps_zero_F)
        {
  	        // only go forward in the direction
  	        return false;
  	    }
	
  	    Vec3 hit_pos = ray.dir * hit_t + ray.pos;
  	    // now check if it is inside the triangle using barycentric coordinates
  	    float a, b, c;
	
  	    Vec3 to_p3 = pos3_ - hit_pos;
  	    Vec3 to_p2 = pos2_ - hit_pos;
  	    Vec3 to_p1 = pos1_ - hit_pos;
		// barycentric coordinate test for triangle intersection - this makes it easy to interpret between vertices
  	    a = Magnitude(Cross(to_p3, to_p2)) / face_norm_len_;
  	    b = Magnitude(Cross(to_p3, to_p1)) / face_norm_len_;
  	    c = Magnitude(Cross(to_p1, to_p2)) / face_norm_len_;
		float one = 1.f + eps_zero_F;
  	    if(a <= one && b <= one && c <= one && a + b + c <= one)
        {
    	    collision_pt.pos = hit_pos;
    	    collision_pt.hit_time = hit_t;
    	
    	    if (use_vertex_norms_)
            {
                collision_pt.norm = norm1_ * a + norm2_ * b + norm3_ * c;
				collision_pt.norm = Normalize(collision_pt.norm);
            }
			else
            {
				collision_pt.norm = face_norm_;
			}
    	
    	    collision_pt.m = mat_;
    	    Vec2 uv = uv1_ * a + uv2_ * b + uv3_ * c;
    	    /*if (use_vertex_uvs_) {
    	        // TODO: Move Texture sampling for normal vectors & base color to after the closest collision has been determined
    	        Vec3 tangent = a * (t1_) + b * (t2_) + c * (t3_);
    	        Vec3 bitangent = a * (bt1_) + b * (bt2_) + c * (bt3_);
    	        Vec3 normal = a * (n1_) + b * (n2_) + c * (n3_);
    	        Mat4 TBN(Vec4(tangent, 0), Vec4(bitangent, 0), Vec4(normal, 0), Vec4(0, 0, 0, 1));
    	        // Get normal from texture
    	        Color norm_clr = mat_->normal_map_->sample(hit.uv.x, hit.uv.y);
    	        Vec4 tex_norm = Vec4(norm_clr.r, norm_clr.g, norm_clr.b) * 2.f - 1.0f;
    	        tex_norm = TBN * tex_norm;
    	        hit.norm = Vec3(tex_norm.x, tex_norm.y, tex_norm.z);
    	    }*/
		    if (Dot(collision_pt.norm, ray.dir) > 0.f)
            {
				// we are exiting this medium, since the normals are facing the same direction 
				collision_pt.medium_ior = mat_->IOR();
				collision_pt.norm = -collision_pt.norm;
			}
			else
            {
				// we are entering the medium, so use 1.0 to represent air for now
				collision_pt.medium_ior = 1.f;
			}
		    return true;
  	    }
  	    return false;
    }

	BoundingBox Triangle::GetBounds()
	{
		std::pair<float, float> x_rng = std::minmax( {pos1_.x, pos2_.x, pos3_.x} );
		std::pair<float, float> y_rng = std::minmax( {pos1_.y, pos2_.y, pos3_.y} );
		std::pair<float, float> z_rng = std::minmax( {pos1_.z, pos2_.z, pos3_.z} );

		return BoundingBox(Vec3(x_rng.first, y_rng.first, z_rng.first), Vec3(x_rng.second, y_rng.second, z_rng.second));
	}
}