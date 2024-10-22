set(COBALT_MATH_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/vec2.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vec3.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vec4.h
    ${CMAKE_CURRENT_SOURCE_DIR}/simd/simd_vec3.h
    ${CMAKE_CURRENT_SOURCE_DIR}/simd/simd_vec4.h
    ${CMAKE_CURRENT_SOURCE_DIR}/simd/simd_mat3.h
    ${CMAKE_CURRENT_SOURCE_DIR}/simd/simd_mat4.h)

target_sources(CobaltCore PUBLIC ${COBALT_MATH_HEADERS})
target_include_directories(CobaltCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})