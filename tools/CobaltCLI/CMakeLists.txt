set(COBALT_CLI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/cli_progress.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cli_progress.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cobalt_cli.cpp)

add_executable(CobaltCLI ${COBALT_CLI_SOURCES})

target_include_directories(CobaltCLI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(commands)

target_link_libraries(CobaltCLI PRIVATE CobaltCore CobaltRender CobaltToolsAssets CobaltToolsUtilities)