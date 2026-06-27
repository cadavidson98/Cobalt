#include <cstddef>
#include <cstdlib>

#include <OSL/oslcomp.h>
#include <OSL/oslexec.h>
#include "OSL/rendererservices.h"
#include <OSL/oslconfig.h>
#include <OSL/shaderglobals.h>

#include <string>
#include <filesystem>
#include <iostream>
#include <iomanip>

class Renderer : public OSL::RendererServices {
public:
virtual ~Renderer() {};
private:
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "invalid argument";
        return EXIT_FAILURE;
    }

    std::filesystem::path inputShaderFile = (argv[1]);

    std::vector<std::string> options;

    OSL::OSLCompiler compiler;
    const bool compiled = compiler.compile(inputShaderFile.c_str(), options);
    if (!compiled) {
        std::cerr << "failed to compile" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "compiled " << compiler.output_filename() << std::endl;

    std::filesystem::path compiledShaderFile = OIIO::c_str(compiler.output_filename());

    Renderer renderer;
    OSL::ShadingSystem shadingSystem(&renderer, NULL, NULL);
    OSL::ShaderGroupRef shaderGroup = shadingSystem.ShaderGroupBegin("basic_shader");

    const OSL::Vec3 baseColor(0.f, 0.f, 1.f);

    shadingSystem.Parameter(*shaderGroup, "base", OSL::TypeColor, &baseColor);
    shadingSystem.Shader("surface", "matte", "matte_surface");
    shadingSystem.ShaderGroupEnd();

    OSL::PerThreadInfo *singletonThreadInfo = shadingSystem.create_thread_info();
    OSL::ShadingContext *context = shadingSystem.get_context(singletonThreadInfo, NULL);
    OSL::ShaderGlobals shaderGlobals;
    const bool executed = shadingSystem.execute(
        *context, 
        *shaderGroup, 
        0, 
        0, 
        shaderGlobals, 
        nullptr, 
        nullptr
    );

    std::cout << "osl run: " << std::boolalpha << executed << std::endl;

    shadingSystem.execute_cleanup(*context);
    shadingSystem.release_context(context);
    shadingSystem.destroy_thread_info(singletonThreadInfo);

    return EXIT_SUCCESS;
}