set(COBALT_CLI_UTILITIES_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image.h)

add_library(CobaltToolsUtilities STATIC ${COBALT_CLI_UTILITIES_SOURCES})
target_include_directories(CobaltToolsUtilities INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CobaltToolsUtilities CobaltRender stbimage)
