add_library(Cobalt INTERFACE)
target_compile_features(Cobalt INTERFACE cxx_std_20)

add_subdirectory(core)
add_subdirectory(render)