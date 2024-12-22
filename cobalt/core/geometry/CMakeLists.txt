set(COBALT_GEOMETRY_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/bounding_box.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bounding_volume.h
    ${CMAKE_CURRENT_SOURCE_DIR}/intersection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mesh.h
    ${CMAKE_CURRENT_SOURCE_DIR}/quad.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ray.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sphere.h)

set(COBALT_GEOMETRY_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/bounding_volume.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/intersection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mesh.cpp)

target_sources(CobaltCore PUBLIC ${COBALT_GEOMETRY_HEADERS} PRIVATE ${COBALT_GEOMETRY_SOURCES})
target_include_directories(CobaltCore AFTER PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})