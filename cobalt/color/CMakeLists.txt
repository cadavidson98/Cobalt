add_library(CobaltColor STATIC)
target_compile_features(CobaltColor PUBLIC cxx_std_20)

set(COBALT_COLOR_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/blackbody_spectrum.h
    ${CMAKE_CURRENT_SOURCE_DIR}/polynomial_spectrum.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pixel_buffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/rgb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sampled_spectrum.h
    ${CMAKE_CURRENT_SOURCE_DIR}/xyz.h)

set(COBALT_COLOR_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/pixel_buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xyz.cpp)

target_compile_definitions(CobaltColor PUBLIC ${COBALT_LOG_LEVELS})
target_sources(CobaltColor PUBLIC ${COBALT_COLOR_HEADERS} ${COBALT_COLOR_SOURCES})
target_include_directories(CobaltColor BEFORE PUBLIC ${COBALT_ROOT_DIR} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CobaltColor PUBLIC CobaltMath CobaltCore)