#include "sdesc_file_loader.h"

#include "mat/cook_torrence.h"
#include "mat/disney_principled.h"

#include "math/vec.h"
#include "math/mat4.h"
#include "math/math_helpers.h"

#include "light/area_light.h"
#include "light/direction_light.h"

#include "geom/triangle_mesh.h"

#include <iostream>
#include <sstream>
#include <memory>

template<class T>
void parseString(std::istream_iterator<T> iter, std::vector<T> &vals)
{
    std::copy(iter, std::istream_iterator<T>(), std::back_inserter(vals));
}

std::shared_ptr<cblt::Scene> SDescFileLoader::LoadScene(std::string file_name)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(file_name.c_str());
    if (!result)
    {
        return nullptr;
    }
    pugi::xpath_node_set cameras = doc.select_nodes("/SDesc/Library_Cameras/camera");
    for (pugi::xpath_node node: cameras)
    {
        pugi::xml_node camera = node.node();
        ProcessCamera(camera);
    }

    pugi::xpath_node_set materials = doc.select_nodes("/SDesc/Library_Materials/Material");
    for (pugi::xpath_node node: materials)
    {
        pugi::xml_node material = node.node();
        std::string mat_type(material.select_node("material_type").node().text().as_string());
        if (!mat_type.compare("Principled Material"))
        {
            ProcessPrincipledMaterial(material);
        }
        else if (!mat_type.compare("Cook Torrence Material"))
        {
            ProcessTorrenceMaterial(material);
        }
    }

    pugi::xpath_node_set geometry = doc.select_nodes("/SDesc/Library_Geometry/geometry");
    for (pugi::xpath_node node: geometry)
    {
        pugi::xml_node geom = node.node();
        ProcessMesh(geom);
    }

    pugi::xpath_node_set lights = doc.select_nodes("/SDesc/Library_Lights/light");
    for (pugi::xpath_node node: lights)
    {
        pugi::xml_node light = node.node();
        ProcessLight(light);
    }

    // construct the scene
    std::vector<std::shared_ptr<cblt::ScenePrim>> s_prims;
    std::vector<std::shared_ptr<cblt::Light>> l_prims;
    pugi::xpath_node_set scene = doc.select_nodes("/SDesc/Scene/Element");
    for (pugi::xpath_node node: scene)
    {
        pugi::xml_node elem_node = node.node();
        // fetch mesh
        std::string id = elem_node.attribute("primitive_id").as_string();
        
        std::vector<float> transform_arr;
    
        std::stringstream parse_mat(elem_node.child("Transform").text().as_string());
        std::istream_iterator<float> mat_iter(parse_mat);
        parseString(mat_iter, transform_arr);
        cblt::Mat4 transform(reinterpret_cast<float*>(transform_arr.data()));
        if (mesh_map_[id] != nullptr)
        {
            s_prims.push_back(std::make_shared<cblt::ScenePrim>(mesh_map_[id], transform));
        }
    }
    
    return std::make_shared<cblt::Scene>(cam_, s_prims, lights_);
}

bool SDescFileLoader::ProcessCamera(pugi::xml_node &camera_node)
{
    pugi::xml_node pos = camera_node.select_node("position").node();
    pugi::xml_node fwd = camera_node.select_node("forward").node();
    pugi::xml_node up = camera_node.select_node("up").node();
    pugi::xml_node fov = camera_node.select_node("FOV_ha").node();
    
    std::array<pugi::xml_node, 3> cam_nodes = { pos, fwd, up };
    std::array<std::vector<float>, 3> cam_vecs;
    std::array<cblt::Vec3, 3> cam_vals;
    
    for(int i = 0; i < cam_nodes.size(); ++i)
    {
        const pugi::char_t * text = cam_nodes[i].first_child().value();
        const char * txt = cam_nodes[i].first_child().text().as_string();
        std::stringstream parse(text);
        std::istream_iterator<float> iter(parse);
        parseString(iter, cam_vecs[i]);
    
        if (cam_vecs[i].size() < 3)
        {
            return false;
        }

        cam_vals[i].x = cam_vecs[i][0];
        cam_vals[i].y = cam_vecs[i][1];
        cam_vals[i].z = cam_vecs[i][2];
    }
    
    // fov
    float fov_val = cblt::toRadians(std::stof(fov.first_child().value()));
    cam_ = cblt::Camera(cam_vals[0], cam_vals[1], cam_vals[2], fov_val);
    return true;
}

bool SDescFileLoader::ProcessPrincipledMaterial(pugi::xml_node &mat_node)
{
    pugi::xml_node clr = mat_node.select_node("Base_Color").node();
    const char * text = clr.first_child().text().as_string();
    std::stringstream parse(text);
    std::istream_iterator<float> iter(parse);
    
    std::string clr_type = clr.attribute("format").as_string();
    std::vector<float> clr_vals;
    std::shared_ptr<Image> base = nullptr;
    Color mat_base;
    if (clr_type.compare("rgba") == 0)
    {
        parseString(iter, clr_vals);
        mat_base = Color(clr_vals[0], clr_vals[1], clr_vals[2]);
    }
    else if (clr_type.compare("texture") == 0)
    {
        std::string file_name = clr.text().as_string();
        base = std::make_shared<Image>(file_name.c_str());
    }
    // get scalar attribs
    float mat_sub = mat_node.select_node("Subsurface").node().text().as_float();
    float mat_met = mat_node.select_node("Metallic").node().text().as_float();
    float mat_spec = mat_node.select_node("Specular").node().text().as_float();
    float mat_spec_tint = mat_node.select_node("Specular_Tint").node().text().as_float();
    float mat_rough = mat_node.select_node("Roughness").node().text().as_float();
    float mat_aniso = mat_node.select_node("Anisotropic").node().text().as_float();
    float mat_shn = mat_node.select_node("Sheen").node().text().as_float();
    float mat_shn_tint = mat_node.select_node("Sheen_Tint").node().text().as_float();
    float mat_clear = mat_node.select_node("Clearcoat").node().text().as_float();
    float mat_clear_coat = mat_node.select_node("Clearcoat_Gloss").node().text().as_float();
    float mat_ior = mat_node.select_node("IOR").node().text().as_float();
    std::shared_ptr<cblt::Material> material = std::make_shared<cblt::DisneyPrincipledMaterial>(mat_base, mat_sub, mat_met, mat_spec, mat_spec_tint, mat_rough, mat_aniso, mat_shn, mat_shn_tint, mat_clear, mat_clear_coat, mat_ior, false);
    if (base != nullptr)
    {
        material->SetBaseImage(base);
    }

    material_map_[std::string(mat_node.attribute("ID").as_string())] = material;
    return true;
}

bool SDescFileLoader::ProcessTorrenceMaterial(pugi::xml_node &mat_node)
{
    pugi::xml_node albedo = mat_node.select_node("Albedo").node();
    pugi::xml_node specular = mat_node.select_node("Specular").node();
    pugi::xml_node emissive = mat_node.select_node("Emissive").node();
    std::array<pugi::xml_node, 3> cam_nodes = { albedo, specular, emissive };
    std::array<std::vector<float>, 3> clr_vecs;
    std::array<Color, 3> clr_vals;
    
    for(int i = 0; i < cam_nodes.size(); ++i)
    {
        const char *txt = cam_nodes[i].text().as_string();
        std::stringstream parse(txt);
        std::istream_iterator<float> iter(parse);
        parseString(iter, clr_vecs[i]);
    
        if (clr_vecs[i].size() < 3)
        {
            return false;
        }

        clr_vals[i].r = clr_vecs[i][0];
        clr_vals[i].g = clr_vecs[i][1];
        clr_vals[i].b = clr_vecs[i][2];
    }
    
    // get scalar attribs
    float mat_ior = mat_node.select_node("IOR").node().text().as_float();
    float mat_metal = mat_node.select_node("Metallic").node().text().as_float();
    float mat_rough = mat_node.select_node("Roughness").node().text().as_float();
    
    std::shared_ptr<cblt::Material> material = std::make_shared<cblt::CookTorrenceMaterial>(clr_vals[0], clr_vals[1], clr_vals[2], mat_ior, mat_rough, mat_metal);
    material_map_[std::string(mat_node.attribute("ID").as_string())] = material;
    return true;
}

bool SDescFileLoader::ProcessMesh(pugi::xml_node &mesh_node)
{
    pugi::xml_node &node_verts = mesh_node.select_node("positions").node();
    pugi::xml_node &node_tris  = mesh_node.select_node("indices").node();
    pugi::xml_node &node_surfs = mesh_node.select_node("surface_materials").node();
    pugi::xml_node &node_mats  = mesh_node.select_node("materials").node();
    // optional
    pugi::xml_node &node_norms = mesh_node.select_node("normals").node();
    pugi::xml_node &node_uvs   = mesh_node.select_node("uvs").node();
    
    std::vector<float> verts_arr, norms_arr, uvs_arr;
    std::vector<int> ind_arr, mat_ind_arr;
    std::vector<std::string> surfs_arr;
    
    std::stringstream parse_vert(node_verts.text().as_string());
    std::istream_iterator<float> vert_iter(parse_vert);
    parseString(vert_iter, verts_arr);
    
    std::stringstream parse_ind(node_tris.text().as_string());
    std::istream_iterator<int> ind_iter(parse_ind);
    parseString(ind_iter, ind_arr);
    
    std::stringstream parse_surfs(node_surfs.text().as_string());
    std::istream_iterator<std::string> surfs_iter(parse_surfs);
    parseString(surfs_iter, surfs_arr);

    std::stringstream parse_mat_ind(node_mats.text().as_string());
    std::istream_iterator<int> mat_ind_iter(parse_mat_ind);
    parseString(mat_ind_iter, mat_ind_arr);

    if (node_norms)
    {
        std::stringstream parse_norm(node_norms.text().as_string());
        std::istream_iterator<float> norm_iter(parse_norm);
        parseString(norm_iter, norms_arr);
    }

    if (node_uvs)
    {
        std::stringstream parse_uv(node_uvs.text().as_string());
        std::istream_iterator<float> uv_iter(parse_uv);
        parseString(uv_iter, uvs_arr);
    }

    std::vector<std::shared_ptr<cblt::Triangle>> mesh;
    int num_tris = ind_arr.size() / 3;
    for (int i = 0; i < num_tris; ++i)
    {
        int id1 = ind_arr[3 * i];
        int id2 = ind_arr[3 * i + 1];
        int id3 = ind_arr[3 * i + 2];
        cblt::Vec3 pos1(verts_arr[3 * id1], verts_arr[3 * id1 + 1], verts_arr[3 * id1 + 2]);
        cblt::Vec3 pos2(verts_arr[3 * id2], verts_arr[3 * id2 + 1], verts_arr[3 * id2 + 2]);
        cblt::Vec3 pos3(verts_arr[3 * id3], verts_arr[3 * id3 + 1], verts_arr[3 * id3 + 2]);
        std::shared_ptr<cblt::Material> &cur_mat = material_map_[surfs_arr[mat_ind_arr[i]]];
        
        if (node_norms && node_uvs)
        {
            cblt::Vec3 norm1(norms_arr[3 * id1], norms_arr[3 * id1 + 1], norms_arr[3 * id1 + 2]);
            cblt::Vec3 norm2(norms_arr[3 * id2], norms_arr[3 * id2 + 1], norms_arr[3 * id2 + 2]);
            cblt::Vec3 norm3(norms_arr[3 * id3], norms_arr[3 * id3 + 1], norms_arr[3 * id3 + 2]);

            cblt::Vec2 uv1(uvs_arr[2 * id1], uvs_arr[2 * id1 + 1]);
            cblt::Vec2 uv2(uvs_arr[2 * id2], uvs_arr[2 * id2 + 1]);
            cblt::Vec2 uv3(uvs_arr[2 * id3], uvs_arr[2 * id3 + 1]);
            mesh.push_back(std::make_shared<cblt::Triangle>(pos1, pos2, pos3, norm1, norm2, norm3, uv1, uv2, uv3, cur_mat));
        }
        else if (node_norms)
        {
            cblt::Vec3 norm1(norms_arr[3 * id1], norms_arr[3 * id1 + 1], norms_arr[3 * id1 + 2]);
            cblt::Vec3 norm2(norms_arr[3 * id2], norms_arr[3 * id2 + 1], norms_arr[3 * id2 + 2]);
            cblt::Vec3 norm3(norms_arr[3 * id3], norms_arr[3 * id3 + 1], norms_arr[3 * id3 + 2]);

            mesh.push_back(std::make_shared<cblt::Triangle>(pos1, pos2, pos3, norm1, norm2, norm3, cur_mat));
        }
        else
        {
            mesh.push_back(std::make_shared<cblt::Triangle>(pos1, pos2, pos3, cur_mat));
        }
    }

    std::shared_ptr<cblt::Geometry> model = std::make_shared<cblt::TriangleMesh>(mesh);
    mesh_map_[std::string(mesh_node.attribute("ID").as_string())] = model;
    return true;
}

bool SDescFileLoader::ProcessLight(pugi::xml_node &light_node)
{
    std::string light_type(light_node.attribute("type").as_string()); 
    if (!light_type.compare("area light"))
    {
        return ProcessAreaLight(light_node);
    }
    else if (!light_type.compare("direction light"))
    {
        return ProcessDirLight(light_node);
    }
}

bool SDescFileLoader::ProcessDirLight(pugi::xml_node &light_node)
{
    pugi::xml_node node_clr = light_node.select_node("color").node();
    pugi::xml_node node_power = light_node.select_node("power").node();
    pugi::xml_node node_angle = light_node.select_node("spread").node();
    pugi::xml_node node_dir = light_node.select_node("direction").node();
    
    std::vector<float> clr, dir;
    float power, angle;

    std::stringstream parse_clr(node_clr.text().as_string()),
                      parse_dir(node_dir.text().as_string());

    std::istream_iterator<float> clr_iter(parse_clr), 
                                 dir_iter(parse_dir);

    parseString(clr_iter, clr);
    parseString(dir_iter, dir);

    power = node_power.text().as_float();
    angle = node_angle.text().as_float();

    Color light_clr(clr[0], clr[1], clr[2]);
    cblt::Vec3 light_dir(dir[0], dir[1], dir[2]);         

    std::shared_ptr<cblt::Light> light = std::make_shared<cblt::DirectionLight>(light_dir, light_clr, power, angle);
    
    lights_.push_back(light);
    return true;
}

bool SDescFileLoader::ProcessAreaLight(pugi::xml_node &light_node)
{
    pugi::xml_node node_clr = light_node.select_node("color").node();
    pugi::xml_node node_length = light_node.select_node("length").node();
    pugi::xml_node node_width = light_node.select_node("width").node();
    pugi::xml_node node_power = light_node.select_node("power").node();
    pugi::xml_node node_pos = light_node.select_node("position").node();
    pugi::xml_node node_dir_x = light_node.select_node("direction_length").node();
    pugi::xml_node node_dir_y = light_node.select_node("direction").node();
    pugi::xml_node node_dir_z = light_node.select_node("direction_width").node();
    
    std::vector<float> clr, pos, dir_x, dir_y, dir_z;
    float length, width, power;

    std::stringstream parse_clr(node_clr.text().as_string()),
                      parse_pos(node_pos.text().as_string()),
                      parse_d_x(node_dir_x.text().as_string()),
                      parse_d_y(node_dir_y.text().as_string()),
                      parse_d_z(node_dir_z.text().as_string());

    std::istream_iterator<float> clr_iter(parse_clr),
                                 pos_iter(parse_pos),
                                 d_x_iter(parse_d_x),
                                 d_y_iter(parse_d_y),
                                 d_z_iter(parse_d_z);

    parseString(clr_iter, clr);
    parseString(pos_iter, pos);
    parseString(d_x_iter, dir_x);
    parseString(d_y_iter, dir_y);
    parseString(d_z_iter, dir_z);

    length = node_length.text().as_float();
    width = node_width.text().as_float();
    power = node_power.text().as_float();

    Color light_clr(clr[0], clr[1], clr[2]);
    cblt::Vec3 light_pos(pos[0], pos[1], pos[2]);
    cblt::Vec3 light_dir(dir_y[0], dir_y[1], dir_y[2]),
               light_l_d(dir_x[0], dir_x[1], dir_x[2]),
               light_w_d(dir_z[0], dir_z[1], dir_z[2]);

    std::shared_ptr<cblt::Light> light = std::make_shared<cblt::AreaLight>(light_pos, light_l_d, light_dir, light_w_d, light_clr, power, length, width);
    
    lights_.push_back(light);
    return true;
}

bool SDescFileLoader::ProcessPrim(pugi::xml_node &elem_node)
{
    return true;
}