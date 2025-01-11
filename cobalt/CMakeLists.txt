add_library(Cobalt STATIC)
target_compile_features(Cobalt PUBLIC cxx_std_20)

add_subdirectory(core)
add_subdirectory(geometry)
add_subdirectory(math)

add_subdirectory(render)

target_link_libraries(Cobalt PUBLIC CobaltCore CobaltGeometry CobaltMath CobaltRender)

add_subdirectory(tests)