add_library(CobaltMath STATIC)
target_compile_features(CobaltMath PUBLIC cxx_std_20)

set(COBALT_MATH_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/constants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interpolation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/math_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mat3.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mat4.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vec2.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vec3.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vec4.h
    ${CMAKE_CURRENT_SOURCE_DIR}/simd/simd_vec3.h
    ${CMAKE_CURRENT_SOURCE_DIR}/simd/simd_vec4.h
    ${CMAKE_CURRENT_SOURCE_DIR}/simd/simd_mat3.h
    ${CMAKE_CURRENT_SOURCE_DIR}/simd/simd_mat4.h
    ${CMAKE_CURRENT_SOURCE_DIR}/math_utilities.h)

set(COBALT_MATH_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/math_utilities.cpp)

target_sources(CobaltMath PUBLIC ${COBALT_MATH_HEADERS} PRIVATE ${COBALT_MATH_SOURCES})
target_include_directories(CobaltMath PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})