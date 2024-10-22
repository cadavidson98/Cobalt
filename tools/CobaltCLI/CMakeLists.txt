set(COBALT_CLI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/argument_parser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/argument_parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cobalt_cli.cpp)

add_executable(CobaltCLI ${COBALT_CLI_SOURCES})

target_link_libraries(CobaltCLI PRIVATE CobaltCore CobaltRender stbimage)