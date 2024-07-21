set(COBALT_GEOMETRY_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/bounding_box.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ray.h)

add_library(CobaltGeometry INTERFACE)
target_compile_features(CobaltGeometry INTERFACE cxx_std_20)
target_sources(CobaltGeometry PUBLIC ${COBALT_GEOMETRY_HEADERS})
target_link_libraries(CobaltGeometry INTERFACE CobaltMath)
target_include_directories(CobaltGeometry INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})