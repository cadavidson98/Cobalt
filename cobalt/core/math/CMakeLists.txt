set(COBALT_MATH_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/simd/simd_vec3.h
    ${CMAKE_CURRENT_SOURCE_DIR}/simd/simd_vec4.h
    ${CMAKE_CURRENT_SOURCE_DIR}/simd/simd_mat3.h
    ${CMAKE_CURRENT_SOURCE_DIR}/simd/simd_mat4.h)

add_library(CobaltMath INTERFACE)
target_compile_features(CobaltMath INTERFACE cxx_std_20)
target_sources(CobaltMath PUBLIC ${COBALT_MATH_HEADERS})
target_include_directories(CobaltMath INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})