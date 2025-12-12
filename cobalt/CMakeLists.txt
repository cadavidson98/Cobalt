add_library(Cobalt STATIC)
target_compile_features(Cobalt PUBLIC cxx_std_20)

set(COBALT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(color)
add_subdirectory(core)
add_subdirectory(geometry)
add_subdirectory(io)
add_subdirectory(math)
add_subdirectory(render)

target_link_libraries(Cobalt PUBLIC CobaltCore CobaltColor CobaltGeometry CobaltIO CobaltMath CobaltRender)
target_include_directories(Cobalt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(tests)