set(COBALT_CLI_UTILITIES_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/image_writer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image_writer.h)

add_library(CobaltToolsUtilities STATIC ${COBALT_CLI_UTILITIES_SOURCES})
target_include_directories(CobaltToolsUtilities INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CobaltToolsUtilities CobaltRender)
