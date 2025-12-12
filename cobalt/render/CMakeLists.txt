add_library(CobaltRender STATIC)

find_package(Imath REQUIRED)
find_package(Ptex REQUIRED)

target_compile_features(CobaltRender PUBLIC cxx_std_20)
target_link_libraries(CobaltRender PUBLIC CobaltCore CobaltMath CobaltGeometry CobaltIO Imath::Imath Ptex)

add_subdirectory(bsdf)
add_subdirectory(data)
add_subdirectory(material)
add_subdirectory(utilities)
add_subdirectory(scene)

set(COBALT_RENDER_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/component_storage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer.h)

set(COBALT_RENDER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer.cpp)

target_include_directories(CobaltRender PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(CobaltRender PUBLIC ${COBALT_RENDER_HEADERS} PRIVATE ${COBALT_RENDER_SOURCES})