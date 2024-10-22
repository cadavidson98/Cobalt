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

target_sources(CobaltCore PUBLIC ${COBALT_MATERIAL_HEADERS})
target_include_directories(CobaltCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})