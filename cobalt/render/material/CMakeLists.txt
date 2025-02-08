set(COBALT_MATERIAL_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/fresnel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ggx_function.h
    ${CMAKE_CURRENT_SOURCE_DIR}/lambertian_diffuse.h
    ${CMAKE_CURRENT_SOURCE_DIR}/material.h
    ${CMAKE_CURRENT_SOURCE_DIR}/microfacet_specular.h
    ${CMAKE_CURRENT_SOURCE_DIR}/smith_geometry_function.h
    ${CMAKE_CURRENT_SOURCE_DIR}/surface_function.h
    ${CMAKE_CURRENT_SOURCE_DIR}/surface_params.h
    )

set(COBALT_MATERIAL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/material.cpp)

target_sources(CobaltRender PUBLIC ${COBALT_MATERIAL_SOURCES} ${COBALT_MATERIAL_HEADERS})
target_include_directories(CobaltRender PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})