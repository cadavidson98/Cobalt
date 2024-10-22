#include "geom\scene.h"
#include "geom\triangle.h"
#include "geom\triangle_mesh.h"
#include "math\vec.h"

#include "bvh.h"

#include <iostream>
#include <fstream>
#include <memory>
#include <string>

cblt::Scene *loadScene(std::string &file_name);

int main(int argc, char* argv[])
{
    std::string file_name = "D:\\Personal Projects\\PathTracer\\data\\dragonEmissive.txt";
    cblt::Scene * test_scene = loadScene(file_name);
    if (test_scene != nullptr) delete test_scene;
    return 0;
}

cblt::Scene *loadScene(std::string &file_name)
{
    std::ifstream in_file(file_name);
    //cblt::Scene *new_scene = new cblt::Scene;
    if (!in_file.good()) 
    {
        return nullptr;
    }
    // text-based OBJ style, can process line-by-line
    std::vector<std::shared_ptr<cblt::Triangle>> triangles;
    std::vector<Triangle> ref_triangles;
    std::vector<cblt::Vec3> verts;
    std::vector<cblt::Vec3> norms;
    // The maximum and minimum bounds of the scene. Used when generating the BVH
    int max_vert(-1), max_norm(-1), nindex(0), vindex(0), tindex(0);
    // The Camera variables
    cblt::Vec3 cam_eye, cam_fwd, cam_up;
    // Read each line in the file
    std::string command, line;
    while (!in_file.eof())
    {
        in_file >> command;
        if (!command.compare("max_vertices:"))
        {
            in_file >> max_vert;
            // Reserve space for the vertices
            verts.reserve(3 * max_vert);
        }
        else if (!command.compare("max_normals:"))
        {
            in_file >> max_norm;
            // Reserve space for the normals
            norms.reserve(3 * max_norm);
            norms.resize(3 * max_norm);
        }
        else if (!command.compare("vertex:"))
        {
            // Add another vertex to the master list
            float vert_x, vert_y, vert_z;
            in_file >> vert_x >> vert_y >> vert_z;
            verts.emplace_back(vert_x, vert_y, vert_z);
        }
        else if (!command.compare("normal:"))
        {
            // Add another normal to the master list
            float norm_x, norm_y, norm_z;
            in_file >> norm_x >> norm_y >> norm_z;
            norms.emplace_back(norm_x, norm_y, norm_z);
        }
        else if (!command.compare("triangle:"))
        {
            int p1, p2, p3;
            in_file >> p1 >> p2 >> p3;
            if(p1 >= verts.size() || p2 >= verts.size() || p3 >= verts.size())
            {
                continue;
            }
            // Find the corresponding vertices in the vertex array and add them to the triangle
            std::shared_ptr<cblt::Triangle> new_tri = std::make_shared<cblt::Triangle>(verts[p1], verts[p2], verts[p3]);
            triangles.push_back(new_tri);
            Triangle ref;
            ref.p1_ = Vec3(verts[p1].x, verts[p1].y, verts[p1].z);
            ref.p2_ = Vec3(verts[p2].x, verts[p2].y, verts[p2].z);
            ref.p3_ = Vec3(verts[p3].x, verts[p3].y, verts[p3].z);
            ref_triangles.push_back(ref);
        }
        else if (!command.compare("normal_triangle:"))
        {
            int p1, p2, p3, n1, n2, n3;
            in_file >> p1 >> p2 >> p3 >> n1 >> n2 >> n3;
            if(p1 >= verts.size() || p2 >= verts.size() || p3 >= verts.size()
            || n1 >= norms.size() || n2 >= norms.size() || n3 >= norms.size())
            {
                continue;
            }
            std::shared_ptr<cblt::Triangle> new_tri = std::make_shared<cblt::Triangle>(verts[p1], verts[p2], verts[p3], norms[n1], norms[n2], norms[n3]);
            triangles.push_back(new_tri);
        }
        else
        {
            // Unsupported command or comment, just skip it
            std::getline(in_file, line);
            continue;
        }
    }
    std::shared_ptr<cblt::TriangleMesh> mesh = std::make_shared<cblt::TriangleMesh>(triangles);
    bvh reference = bvh(ref_triangles);
    std::vector<std::shared_ptr<cblt::ScenePrim>> prims;
    bool same = true;

    prims.push_back(std::make_shared<cblt::ScenePrim>(mesh, cblt::Identity_F));
    return new cblt::Scene(cblt::Camera(), prims);
}