#include <fstream>
#include <iostream>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "config.h"
#include "camera.h"
#include "image_lib.h"
#include "legacy_file_loader.h"
#include "sdesc_file_loader.h"
#include "ray_tracer.h"

bool loadConfiguration(std::string &file_path, RenderSettings &settings);

int main(int argc, char* argv[])
{
    RenderSettings settings = { 1280, 720, 32, 8, 10, 32 };
    std::string file_name = std::string(DEBUG_DIR) + '/'; 
    std::string out_name = std::string(DEBUG_DIR) + '/';
    for (int i = 1; i < argc - 1; ++i)
    {
        std::string arg = argv[i];
        if (!arg.compare("--conf") || !arg.compare("-c"))
        {
            std::string conf_file = std::string(DEBUG_DIR) + '/' + argv[++i];
            // load params from configuration file
            if (!loadConfiguration(conf_file, settings))
            {
                std::cerr << "Invalid Configuration File provided" << std::endl;
                std::exit(1);
            }
        }
        else if (!arg.compare("--input") || !arg.compare("-i"))
        {
            // load input file
            file_name += argv[++i];
        }
        else if (!arg.compare("--output") || !arg.compare("-o"))
        {
            // write output file
            out_name += argv[++i];
        }
        else if (!arg.compare("--width") || !arg.compare("-w"))
        {
            // set image size
            settings.img_width = std::stoi(argv[++i]);
        }
        else if (!arg.compare("--height") || !arg.compare("-h"))
        {
            // set image size
            settings.img_height = std::stoi(argv[++i]);
        }
    }
    
    std::cout << "Opening " << file_name << std::endl;
    std::shared_ptr<cblt::Scene> my_scene;
    if (file_name.find(".txt") != std::string::npos)
    {
        // legacy files from CSCI 5607 format
        std::cout << "Legacy format detected\n";
        LegacyFileLoader file_loader;
        my_scene = file_loader.LoadScene(file_name);
    }
    else if (file_name.find(".sdesc") != std::string::npos)
    {
        std::cout << "SDesc format detected\n";
        SDescFileLoader file_loader;
        my_scene = file_loader.LoadScene(file_name);
    }
    else
    {
        std::cerr << "Invalid file format" << std::endl;
        std::exit(1);
    }
    
    my_scene->cam_.ConfigureExtent(static_cast<float>(settings.img_width), static_cast<float>(settings.img_height));
    RayTracer ray_tracer(settings, my_scene);
    auto s_time = std::chrono::high_resolution_clock::now();
    std::shared_ptr<Image> img = ray_tracer.Render();
    auto e_time = std::chrono::high_resolution_clock::now();
    
    auto mil = std::chrono::duration_cast<std::chrono::milliseconds>(e_time - s_time);
    int minutes = static_cast<int>(mil.count() / 60000);
    int seconds = static_cast<int>(mil.count() / 1000 - 60 * minutes);
    int milliseconds = static_cast<int>(mil.count() - 60000 * minutes - 1000 * seconds); 
    std::cout << "\nRender Time " << minutes << ":" << seconds << "." << milliseconds << "\n"; 
    /*std::cout << "Change output file name (y/n)? ";
    char new_file_name;
    std::cin >> new_file_name;
    if (new_file_name == 'y' || new_file_name == 'Y')
    {
        std::cout << "Enter new file name: ";
        std::cin >> out_name;
        out_name = std::string(DEBUG_DIR) + '/' + out_name; 
    }*/
    img->write(out_name.c_str());
    std::cout << "Wrote result to " << out_name << std::endl;
    return 0;
}

bool loadConfiguration(std::string &file_path, RenderSettings &settings)
{
    std::ifstream fin(file_path);
    if (!fin.good())
    {
        return false;
    }

    std::string line;
    while (!fin.eof())
    {
        fin >> line;
        if (!line.compare("width:"))
        {
            fin >> settings.img_width;
        }
        else if (!line.compare("height:"))
        {
            fin >> settings.img_height;
        }
        else if (!line.compare("path_depth:"))
        {
            fin >> settings.path_depth;
        }
        else if (!line.compare("num_samples:"))
        {
            fin >> settings.num_samples;
        }
        else if (!line.compare("tile_size:"))
        {
            fin >> settings.tile_size;
        }
        else if (!line.compare("num_threads:"))
        {
            fin >> settings.num_threads;
        }
    }
    
    return true;
}