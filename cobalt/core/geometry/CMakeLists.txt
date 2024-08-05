set(COBALT_GEOMETRY_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/bounding_box.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bounding_volume.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ray.h)

set(COBALT_GEOMETRY_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/bounding_volume.cpp)

add_library(CobaltGeometry STATIC)
target_compile_features(CobaltGeometry INTERFACE cxx_std_20)
target_sources(CobaltGeometry PUBLIC ${COBALT_GEOMETRY_HEADERS} PRIVATE ${COBALT_GEOMETRY_SOURCES})
target_link_libraries(CobaltGeometry PRIVATE CobaltMath)
target_include_directories(CobaltGeometry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})