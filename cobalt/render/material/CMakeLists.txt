set(COBALT_MATERIAL_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/fresnel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ggx_function.h
    ${CMAKE_CURRENT_SOURCE_DIR}/smith_geometry_function.h
    )

target_sources(CobaltRender PUBLIC ${COBALT_MATERIAL_HEADERS})
target_include_directories(CobaltRender PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})