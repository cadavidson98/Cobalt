add_library(CobaltRender STATIC)

find_package(PNG REQUIRED)
find_package(OpenEXR REQUIRED)
find_package(Ptex REQUIRED)

target_compile_features(CobaltRender PUBLIC cxx_std_20)
target_include_directories(CobaltRender PRIVATE ${OpenEXR_includedir})
target_link_libraries(CobaltRender PUBLIC CobaltCore CobaltMath CobaltGeometry OpenEXR::OpenEXR Ptex png)

add_subdirectory(bsdf)
add_subdirectory(data)
add_subdirectory(material)
add_subdirectory(utilities)
add_subdirectory(scene)