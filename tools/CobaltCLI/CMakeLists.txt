set(COBALT_CLI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/cobalt_cli.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/debug_builder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/debug_builder.cpp)

add_executable(CobaltCLI ${COBALT_CLI_SOURCES})

target_include_directories(CobaltCLI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(commands)

target_link_libraries(CobaltCLI PRIVATE CobaltCore CobaltRender CobaltToolsAssets CobaltToolsUtilities)