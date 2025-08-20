add_library(CobaltGeometry STATIC)
target_compile_features(CobaltGeometry PUBLIC cxx_std_20)

set(COBALT_GEOMETRY_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/bounding_box.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bounding_volume.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bounding_volume_crtp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bounding_volume_crtp.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/bounding_volume_scene_storage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bounding_volume_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/intersection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mesh.h
    ${CMAKE_CURRENT_SOURCE_DIR}/quad.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ray.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sphere.h)

set(COBALT_GEOMETRY_SOURCES
    # ${CMAKE_CURRENT_SOURCE_DIR}/bounding_volume.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/intersection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mesh.cpp)

message(STATUS "Cobalt root dir: ${COBALT_ROOT_DIR}")

target_sources(CobaltGeometry PUBLIC ${COBALT_GEOMETRY_HEADERS} PRIVATE ${COBALT_GEOMETRY_SOURCES})
target_include_directories(CobaltGeometry PUBLIC ${COBALT_ROOT_DIR} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}) 
target_link_libraries(CobaltGeometry PUBLIC CobaltMath CobaltCore)