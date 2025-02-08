set(COBALT_RENDER_UTILITIES_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/texture_utilities.h)

target_sources(CobaltRender PUBLIC ${COBALT_RENDER_UTILITIES_HEADERS})
target_include_directories(CobaltRender AFTER PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})