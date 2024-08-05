add_library(Cobalt INTERFACE)
target_compile_features(Cobalt INTERFACE cxx_std_20)

target_link_libraries(Cobalt INTERFACE CobaltMath)
add_subdirectory(core)
# add_subdirectory(geom)
# add_subdirectory(utils)