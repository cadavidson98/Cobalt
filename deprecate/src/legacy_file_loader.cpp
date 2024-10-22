#include "legacy_file_loader.h"

#include "mat/cook_torrence.h"
#include "mat/disney_principled.h"

#include "math/vec.h"
#include "math/constants.h"
#include "math/math_helpers.h"

#include "light/area_light.h"

#include "geom/triangle.h"
#include "geom/triangle_mesh.h"
#include "geom/scene_prim.h"

#include <fstream>
#include <memory>

std::shared_ptr<cblt::Scene> LegacyFileLoader::LoadScene(std::string file_name) {
    std::ifstream in_file(file_name);
    
    if (!in_file.good()) {
        return nullptr;
    }
    // text-based OBJ style, can process line-by-line
    // The default material is a matte white
    int num_mats = 1;
    std::shared_ptr<cblt::Material> cur_mat = std::make_unique<cblt::CookTorrenceMaterial>(
            Color(1.f, 1.f, 1.f), 
            Color(1.f, 1.f, 1.f), 
            Color(0.f, 0.f, 0.f),
            1.f, 1.f, 0.0f);
    
    std::vector<std::shared_ptr<cblt::Light>> lights;
    
    std::vector<std::shared_ptr<cblt::Triangle>> tris;
    std::vector<cblt::Vec3> verts;
    std::vector<cblt::Vec3> norms;
    // The maximum and minimum bounds of the scene. Used when generating the BVH
    int max_vert(0), max_norm(0), nindex(0), vindex(0), tindex(0);
    // The Camera variables
    cblt::Vec3 cam_eye, cam_fwd, cam_up;
    float cam_FOV, cam_dist;
    // Read each line in the file
    std::string command, line;
    while (!in_file.eof()) {
        in_file >> command;
        if (!command.compare("camera_fwd:")) {
            in_file >> cam_fwd.x >> cam_fwd.y >> cam_fwd.z;
        }
        else if (!command.compare("camera_up:")) {
            in_file >> cam_up.x >> cam_up.y >> cam_up.z;
        }
        else if (!command.compare("camera_pos:")) {
            in_file >> cam_eye.x >> cam_eye.y >> cam_eye.z;
        }
        else if (!command.compare("camera_fov_ha:")) {
            in_file >> cam_FOV;
            cam_FOV = cblt::toRadians(cam_FOV); // convert to radians
        }
        else if (!command.compare("camera_near:")) {
            in_file >> cam_dist;
        }
        else if (!command.compare("material:")) {
            Color albedo, specular, emissive;
            float ior(1.f), rough(0.f), metal(0.f);
            in_file >> albedo.r >> albedo.g >> albedo.b
                    >> specular.r >> specular.g >> specular.b
                    >> emissive.r >> emissive.g >> emissive.b
                    >> ior >> rough >> metal;
            if (emissive.r + emissive.g + emissive.b > 0.f || metal == 0.f)
            {
                cur_mat = std::make_unique<cblt::CookTorrenceMaterial>(albedo, specular, emissive, ior, rough, metal);
            }
            else
            {
                Color base(albedo.r, albedo.g, albedo.b);
                cur_mat = std::make_unique<cblt::DisneyPrincipledMaterial>(base, 0.f, metal, 0.f, 0.f, rough, 1.f, 0.f, 0.f, 0.f, 0.f, 1.0, false);
            }            
        }
        else if (!command.compare("max_vertices:")) {
            in_file >> max_vert;
            // Reserve space for the vertices
            verts.reserve(max_vert);
        }
        else if (!command.compare("max_normals:")) {
            in_file >> max_norm;
            // Reserve space for the normals
            norms.reserve(max_norm);
        }
        else if (!command.compare("vertex:")) {
            // Add another vertex to the master list
            float vert_x, vert_y, vert_z;
            in_file >> vert_x >> vert_y >> vert_z;
            verts.emplace_back(vert_x, vert_y, vert_z);
        }
        else if (!command.compare("normal:")) {
            // Add another normal to the master list
            float norm_x, norm_y, norm_z;
            in_file >> norm_x, norm_y, norm_z;
            norms.emplace_back(norm_x, norm_y, norm_z);
        }
        else if (!command.compare("triangle:")) {
            int p1, p2, p3;
            cblt::Triangle new_tri;
            in_file >> p1 >> p2 >> p3;
            if(p1 >= verts.size() || p2 >= verts.size() || p3 >= verts.size()) {
                continue;
            }
            // Find the corresponding vertices in the vertex array and add them to the triangle
            tris.push_back(std::make_shared<cblt::Triangle>(verts[p1], verts[p2], verts[p3], cur_mat));
            //new_tri.mat_ = cur_mat;
        }
        else if (!command.compare("normal_triangle:")) {
            int p1, p2, p3, n1, n2, n3;
            in_file >> p1 >> p2 >> p3 >> n1 >> n2 >> n3;
            if(p1 >= verts.size() || p2 >= verts.size() || p3 >= verts.size()
            || n1 >= norms.size() || n2 >= norms.size() || n3 >= norms.size()) {
                continue;
            }
            //new_tri.mat_ = cur_mat;
            tris.push_back(std::make_shared<cblt::Triangle>(verts[p1], verts[p2], verts[p3], norms[n1], norms[n2], norms[n3], cur_mat));
        }
        /*else if (!command.compare("point_light:")) {
            Vec4 pos(0.f, 0.f, 0.f, 1.f);
            Vec3 falloff(0.f, 0.f, 1.f);
            Color clr;
            in_file >> clr.r >> clr.g >> clr.b
                    >> pos.x >> pos.y >> pos.z;
            Light *pt_light = new Light{LightType::POINT, clr, pos, falloff, Vec3(), Vec3(), 0.f, 0.f};
            new_scene->lights_.push_back(pt_light);
        }
        else if (!command.compare("directional_light:")) {
            Vec4 pos(0.f);
            Color clr;
            Vec3 falloff;
            in_file >> clr.r >> clr.g >> clr.b
                    >> pos.x >> pos.y >> pos.z;
            Light *dir_light = new Light{LightType::DIR, clr, pos, falloff, Vec3(), Vec3(), 0.f, 0.f};
            new_scene->lights_.push_back(dir_light);
        }*/
        else if(!command.compare("quad_light:")) {
            Color clr;
            cblt::Vec3 pos, dims, left, up, right;
            in_file >> clr.r >> clr.g >> clr.b
                    >> pos.x >> pos.y >> pos.z
                    >> dims.x >> dims.y
                    >> left.x >> left.y >> left.z
                    >> up.x >> up.y >> up.z;
            right = Normalize(cblt::Cross(left, up));
            std::shared_ptr<cblt::Light> quad_light = std::make_shared<cblt::AreaLight>(pos, left, up, right, clr, 1.0f, dims.x, dims.y);
            lights.push_back(quad_light);
        }
        else {
            // Unsupported command or comment, just skip it
            std::getline(in_file, line);
            continue;
        }
    }
    // store all the triangles in a single mesh
    std::shared_ptr<cblt::Geometry> geom = std::make_shared<cblt::TriangleMesh>(tris);
    std::vector<std::shared_ptr<cblt::ScenePrim>> mesh;
    mesh.push_back(std::make_shared<cblt::ScenePrim>(geom, cblt::Identity_F));
    
    // create the camera
    cblt::Camera cam(cam_eye, cam_fwd, cam_up, cam_FOV);

    // construct the scene
    std::shared_ptr<cblt::Scene> new_scene = std::make_shared<cblt::Scene>(cam, mesh, lights);
    return new_scene;
}