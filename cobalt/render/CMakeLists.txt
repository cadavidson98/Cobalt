add_library(CobaltRender STATIC)

find_package(OpenEXR REQUIRED)

target_compile_features(CobaltRender PUBLIC cxx_std_20)
target_include_directories(CobaltRender PRIVATE ${OpenEXR_includedir})
target_link_libraries(CobaltRender PUBLIC CobaltCore OpenEXR::OpenEXR)

add_subdirectory(data)
add_subdirectory(utilities)
add_subdirectory(tests)
add_subdirectory(scene)