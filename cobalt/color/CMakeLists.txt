add_library(CobaltColor STATIC)
target_compile_features(CobaltColor PUBLIC cxx_std_20)

set(CobaltColorAssetsDir ${CMAKE_CURRENT_SOURCE_DIR}/data)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets.h.in ${CMAKE_CURRENT_SOURCE_DIR}/assets.h)

set(COBALT_COLOR_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/color_space_xyz.h
    ${CMAKE_CURRENT_SOURCE_DIR}/spectrum.h)

set(COBALT_COLOR_PRIVATE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/assets.h)

set(COBALT_COLOR_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/color_space_xyz.cpp)

target_sources(CobaltColor PUBLIC ${COBALT_COLOR_HEADERS} PRIVATE ${COBALT_COLOR_SOURCES})
target_include_directories(CobaltColor BEFORE PUBLIC ${COBALT_ROOT_DIR} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CobaltColor PUBLIC CobaltMath CobaltCore)